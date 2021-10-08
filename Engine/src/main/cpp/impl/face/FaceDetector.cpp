//
// Created by liuyuzhou on 2021/9/28.
//
#include <unistd.h>
#include <sys/stat.h>
#include <cstdio>

#include "FaceDetector.h"
#include "Point.h"
#include "png.h"
#include "JniUtil.h"
#include "Common.h"
#include "LogUtil.h"

#define TAG "FaceDetector"
#define CLASSIFIER_PATH "/storage/emulated/0/Android/data/com.render.demo/files/Documents/lbpcascade_frontalface_improved.xml"
#define FACE_LANDMARK_PATH "/storage/emulated/0/Android/data/com.render.demo/files/Documents/shape_predictor_68_face_landmarks.dat"
#define JAVA_CLASS_RECTF "android/graphics/RectF"
#define JAVA_CLASS_LANDMARK "com/render/engine/face/LandMark"

static std::map<std::string, jobject> gClassMap;
const char* rootPath = reinterpret_cast<const char *>("/storage/emulated/0/testImage");

FaceDetector::FaceDetector() {
    mMessageQueue = new ObjectQueue<EventMessage>;
    pthread_mutex_init(&mQuitMutexLock, nullptr);
    pthread_cond_init(&mQuitCondLock, nullptr);

    pthread_mutex_init(&mStatusLock, nullptr);
}

FaceDetector::FaceDetector(ValidPtr<_jobject> *listener) {
    mMessageQueue = new ObjectQueue<EventMessage>;
    mJavaListener = (listener != nullptr && listener->alive()) ? listener : nullptr;
    pthread_mutex_init(&mQuitMutexLock, nullptr);
    pthread_cond_init(&mQuitCondLock, nullptr);

    pthread_mutex_init(&mStatusLock, nullptr);
}

FaceDetector::~FaceDetector() {
    delete mMessageQueue;
    delete mImgQueue;
    mImgQueue = nullptr;
    mMessageQueue = nullptr;
    mStatus = render::Status::STATUS_IDLE;
    //just set the listener null, it will be released outside of FaceDetector
    mJavaListener = nullptr;
    mTrackStopCallback = nullptr;
    mTrackStartCallback = nullptr;
    mFaceDetectCallback = nullptr;
    mCallback = nullptr;
    pthread_mutex_destroy(&mQuitMutexLock);
    pthread_cond_destroy(&mQuitCondLock);

    pthread_mutex_destroy(&mStatusLock);
}

void *processLoop(void *args) {
    auto *detector = static_cast<FaceDetector *>(args);
    JNIEnv *env = nullptr;
    if (!JniUtil::threadAttachJvm(render::gJvm, &env)) {
        LogUtil::logI(TAG, {"renderLoop: failed to attach thread to jvm"});
        return nullptr;
    }
    detector->loop(env);
    JniUtil::detachThread(render::gJvm);
    return nullptr;
}

bool FaceDetector::registerSelf(JNIEnv *env) {
    jclass rectClass = env->FindClass(JAVA_CLASS_RECTF);
    if (rectClass != nullptr) {
        gClassMap.insert(std::pair<std::string , jobject>(JAVA_CLASS_RECTF, env->NewGlobalRef(rectClass)));
    } else {
        return false;
    }

    jclass landmarkClazz = env->FindClass(JAVA_CLASS_LANDMARK);
    if (landmarkClazz != nullptr) {
        gClassMap.insert(std::pair<std::string , jobject>(JAVA_CLASS_LANDMARK, env->NewGlobalRef(landmarkClazz)));
        return true;
    } else {
        return false;
    }
}

void FaceDetector::buildTracker() {
    releaseTracker();
    cv::Ptr<cv::CascadeClassifier> mainClassifier = cv::makePtr<cv::CascadeClassifier>(CLASSIFIER_PATH);
    cv::Ptr<CascadeDetectorAdapter> mainDetector = cv::makePtr<CascadeDetectorAdapter>(mainClassifier);

    cv::Ptr<cv::CascadeClassifier> subClassifier = cv::makePtr<cv::CascadeClassifier>(CLASSIFIER_PATH);
    cv::Ptr<CascadeDetectorAdapter> subDetector = cv::makePtr<CascadeDetectorAdapter>(subClassifier);

    cv::DetectionBasedTracker::Parameters params;
    mFaceTracker = cv::makePtr<cv::DetectionBasedTracker>(mainDetector, subDetector, params);
    bool res = mFaceTracker->run();
    dlib::deserialize(FACE_LANDMARK_PATH) >> mShapePredictor;
    LogUtil::logI(TAG, {"buildTracker: init face tracker, result = ", (res ? "true" : "false")});
}

void FaceDetector::changeStatus(render::Status newState) {
    if (newState >= render::Status::STATUS_IDLE && newState <= render::Status::STATUS_DESTROY) {
        pthread_mutex_lock(&mStatusLock);
        mStatus = newState;
        pthread_mutex_unlock(&mStatusLock);
    }
}

void FaceDetector::enqueueImg(unsigned char *data, int width, int height, int channel, EventType type) {
    if (data != nullptr) {
        std::shared_ptr<Image> img = std::make_shared<Image>();
        img->width = width;
        img->height = height;
        img->channel = channel;
        img->data = static_cast<unsigned char *>(malloc(width * height * channel));
        memcpy(img->data, data, width * height * channel);
        mImgQueue->enqueue(img);
        mMessageQueue->enqueue(EventMessage(type));
    }
}

render::Status FaceDetector::getStatus() {
    render::Status result;
    pthread_mutex_lock(&mStatusLock);
    result = mStatus;
    pthread_mutex_unlock(&mStatusLock);
    return result;
}

bool FaceDetector::isRunning() {
    return getStatus() == render::Status::STATUS_RUN;
}

void FaceDetector::loop(JNIEnv *env) {
    changeStatus(render::Status::STATUS_PREPARING);
    buildTracker();
    changeStatus(render::Status::STATUS_PREPARED);
    LogUtil::logI(TAG, {"loop: prepared"});
    for (;;) {
        EventMessage msg = mMessageQueue->dequeue();
        render::Status curState = getStatus();
        if (msg.what != EventType::EVENT_CHANGE_STATUS && msg.what != EventType ::EVENT_QUIT && curState != render::Status::STATUS_RUN) {
            LogUtil::logI(TAG, {"loop: state is not running, ignore msg ", std::to_string(static_cast<int>(msg.what))});
            mImgQueue->clear();
            continue;
        }
        switch (msg.what) {
            case EventType::EVENT_WRITE_PNG: {
                std::shared_ptr<Image> img = mImgQueue->dequeue();
                if (img->data != nullptr) {
                    writePngImage(img->data, img->width, img->height, img->channel);
                }
                break;
            }
            case EventType::EVENT_FACE_TRACK: {
                std::shared_ptr<Image> img = mImgQueue->dequeueNotWait();
                if (img != nullptr && img->data != nullptr) {
                    trackFaceNative(env, img->data, img->width, img->height, img->channel);
                } else {
                    LogUtil::logI(TAG, {"loop: track face invalid data"});
                }
                break;
            }
            case EventType::EVENT_CHANGE_STATUS: {
                LogUtil::logI(TAG, {"loop: handle change status, new state = ", std::to_string(msg.arg0)});
                auto newState = static_cast<render::Status>(msg.arg0);
                changeStatus(newState);
                if (newState == render::Status::STATUS_RUN) {
                    notifyStartTrack(env);
                } else if (newState == render::Status::STATUS_PAUSE) {
                    notifyStopTrackFace(env);
                }
                break;
            }
            case EventType::EVENT_QUIT: {
                LogUtil::logI(TAG, {"loop: handle quit"});
                changeStatus(render::Status::STATUS_DESTROY);
                notifyStopTrackFace(env);
                goto quit;
            }
            default: {
                break;
            }
        }
    }
    quit:
    releaseTracker();
    mMessageQueue->notify();
    pthread_mutex_lock(&mQuitMutexLock);
    pthread_cond_signal(&mQuitCondLock);
    pthread_mutex_unlock(&mQuitMutexLock);
    LogUtil::logI(TAG, {"loop: quit completely"});
}

void FaceDetector::notifyStartTrack(JNIEnv *env) {
    if (mTrackStartCallback != nullptr) {
        mTrackStartCallback(mCallback);
    }
    if (mJavaListener != nullptr && mJavaListener->alive()) {
        jclass clazz = env->GetObjectClass(mJavaListener->get());
        jmethodID method = env->GetMethodID(clazz, "onTrackStart", "()V");
        env->CallVoidMethod(mJavaListener->get(), method);
    }
}

void FaceDetector::notifyStopTrackFace(JNIEnv *env) {
    if (mTrackStopCallback != nullptr) {
        mTrackStopCallback(mCallback);
    }
    if (mJavaListener != nullptr && mJavaListener->alive()) {
        jclass clazz = env->GetObjectClass(mJavaListener->get());
        jmethodID method = env->GetMethodID(clazz, "onTrackStop", "()V");
        env->CallVoidMethod(mJavaListener->get(), method);
    }
}

void FaceDetector::notifyLandMarkDetect(JNIEnv *env, jclass listenerClass,
        const std::vector<cv::Rect> &faces, const dlib::cv_image<dlib::bgr_pixel> &bgrPixel) {
    dlib::rectangle dFaceRect;
    jclass landMarkClass = static_cast<jclass>(JniUtil::find(&gClassMap, JAVA_CLASS_LANDMARK));
    jmethodID constructMethod = env->GetMethodID(landMarkClass, "<init>", "()V");
    jmethodID setDataMethod = env->GetMethodID(landMarkClass, "setPoint", "(IFF)V");
    jmethodID notifyMethod = env->GetMethodID(listenerClass, "onLandmarkDetect", "([Lcom/render/engine/face/LandMark;)V");
    jobjectArray detectedLandmarks = env->NewObjectArray(faces.size(), landMarkClass, nullptr);
    for (int i = 0; i < faces.size(); i++) {
        dFaceRect.set_left(faces[i].x);
        dFaceRect.set_top(faces[i].y);
        dFaceRect.set_right(faces[i].x + faces[i].width);
        dFaceRect.set_bottom(faces[i].y + faces[i].height);
        dlib::full_object_detection detection = mShapePredictor(bgrPixel, dFaceRect);
        jobject landMark = env->NewObject(landMarkClass, constructMethod);
        for (int j = 0; j < detection.num_parts(); ++j) {
            env->CallVoidMethod(landMark, setDataMethod,
                                j,
                                (jfloat)(detection.part(j).x()),
                                (jfloat)(detection.part(j).y()));
        }
        env->SetObjectArrayElement(detectedLandmarks, i, landMark);
        env->DeleteLocalRef(landMark);
    }
    env->CallVoidMethod(mJavaListener->get(), notifyMethod, detectedLandmarks);
    env->DeleteLocalRef(detectedLandmarks);
}

void FaceDetector::pause() {
    mMessageQueue->enqueue(EventMessage(EventType::EVENT_CHANGE_STATUS, static_cast<int>(render::Status::STATUS_PAUSE)));
}

void FaceDetector::prepare(JNIEnv* env) {
    render::getJvm(env);
    if (isRunning()) {
        LogUtil::logI(TAG, {"prepare: still run status"});
    } else {
        LogUtil::logI(TAG, {"prepare: create thread"});
        if (mImgQueue != nullptr) { mImgQueue->notify(); }
        delete mImgQueue;
        mImgQueue = new PointerQueue<Image>;
        pthread_t thread;
        pthread_create(&thread, nullptr, processLoop, this);
    }
}

void FaceDetector::writePngImage(const unsigned char *data, int width, int height, int channel) {
    if (data == nullptr) {
        LogUtil::logI(TAG, {"writePngImage: input is null"});
        return;
    }
    if (access(rootPath, 0) == -1) { mkdir(rootPath, 0666); }
    char imgPath[256] = {0};
    char pngTitle[256] = {0};
    sprintf(imgPath, "%s/IMG_%lld.%s", rootPath, render::getCurrentTimeMs(), "png");
    sprintf(pngTitle, "%s/IMG_%lld", rootPath, render::getCurrentTimeMs());

    FILE *dst = fopen(imgPath, "wb");
    png_structp pngStruct = nullptr;
    png_infop pngInfo = nullptr;
    png_bytep row = nullptr;
    png_text title_text;
    if (dst == nullptr) {
        LogUtil::logI(TAG, {"writePngImage: failed to open file ", imgPath});
        goto end;
    }
    pngStruct = png_create_write_struct(PNG_LIBPNG_VER_STRING, nullptr, nullptr, nullptr);
    if (pngStruct == nullptr) {
        LogUtil::logI(TAG, {"writePngImage: failed to create png struct ", imgPath});
        goto end;
    }
    pngInfo = png_create_info_struct(pngStruct);
    if (pngInfo == nullptr) {
        LogUtil::logI(TAG, {"writePngImage: failed to create png info ", imgPath});
        goto end;
    }
    if(setjmp(png_jmpbuf(pngStruct))) {
        LogUtil::logI(TAG, {"writePngImage: error during creation"});
        goto end;
    }
    png_init_io(pngStruct, dst);
    png_set_IHDR(pngStruct, pngInfo, width, height, 8, PNG_COLOR_TYPE_RGBA,
            PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_BASE, PNG_FILTER_TYPE_BASE);
    title_text.compression = PNG_TEXT_COMPRESSION_NONE;
    title_text.key = "Title";
    title_text.text = pngTitle;
    png_set_text(pngStruct, pngInfo, &title_text, 1);
    png_write_info(pngStruct, pngInfo);

    row = (png_bytep)malloc(channel * width * sizeof(png_byte));

    int x, y;
    for (y = 0; y < height; y++) {
        for (x = 0; x < width; x++) {
            row[x * channel + 0] = data[y * width * channel + x * channel + 0];
            row[x * channel + 1] = data[y * width * channel + x * channel + 1];
            row[x * channel + 2] = data[y * width * channel + x * channel + 2];
            row[x * channel + 3] = data[y * width * channel + x * channel + 3];
        }
        png_write_row(pngStruct, row);
    }
    png_write_end(pngStruct, nullptr);
    goto end;

    end:
    if (dst != nullptr) { fclose(dst);}
    if (pngInfo != nullptr) { png_free_data(pngStruct, pngInfo, PNG_FREE_ALL, -1); }
    if (pngStruct != nullptr) { png_destroy_write_struct(&pngStruct, nullptr); }
    if (row != nullptr) { free(row); }
}

void FaceDetector::quitAndWait() {
    render::Status status = getStatus();
    if (status < render::Status::STATUS_DESTROY) {
        pthread_mutex_lock(&mQuitMutexLock);
        mMessageQueue->enqueue(EventMessage(EventType::EVENT_QUIT));
        LogUtil::logI(TAG, {"quitAndWait: wait"});
        pthread_cond_wait(&mQuitCondLock, &mQuitMutexLock);
        LogUtil::logI(TAG, {"quitAndWait: resume"});
    }
}

void FaceDetector::releaseTracker() {
    if (mFaceTracker != nullptr) {
        mFaceTracker->stop();
        mFaceTracker.reset();
        mFaceTracker = nullptr;
    }
}

void FaceDetector::start() {
    mMessageQueue->enqueue(EventMessage(EventType::EVENT_CHANGE_STATUS, static_cast<int>(render::Status::STATUS_RUN)));
}

void FaceDetector::trackFace(JNIEnv* env, unsigned char *data, int width, int height, int channel) {
    if (mJavaListener == nullptr || !mJavaListener->alive()) {
        LogUtil::logI(TAG, {"trackFace: invalid listener"});
        return;
    }
    cv::Mat src(height, width, CV_8UC4, data);
    cv::Mat gray, bgrMat;
    cv::cvtColor(src, gray, cv::COLOR_RGBA2GRAY);
    cv::cvtColor(src, bgrMat, cv::COLOR_RGBA2BGR);
    dlib::cv_image<dlib::bgr_pixel> bgrPixel(bgrMat);

    cv::equalizeHist(gray, gray);
    mFaceTracker->process(gray);
    std::vector<cv::Rect> faces;
    mFaceTracker->getObjects(faces);

    jclass listenerClass = env->GetObjectClass(mJavaListener->get());
    if (faces.empty()) {
        LogUtil::logI(TAG, {"trackFace: no face detected"});
        jmethodID method = env->GetMethodID(listenerClass, "onNoFaceDetect", "()V");
        env->CallVoidMethod(mJavaListener->get(), method);
    } else {
        notifyLandMarkDetect(env, listenerClass, faces, bgrPixel);
    }
}

void FaceDetector::trackFaceNative(JNIEnv *env, unsigned char *data, int width, int height, int channel) {
    cv::Mat src(height, width, CV_8UC4, data);
    cv::Mat gray, bgrMat;
    cv::cvtColor(src, gray, cv::COLOR_RGBA2GRAY);
    cv::cvtColor(src, bgrMat, cv::COLOR_RGBA2BGR);
    dlib::cv_image<dlib::bgr_pixel> bgrPixel(bgrMat);

    cv::equalizeHist(gray, gray);
    mFaceTracker->process(gray);
    std::vector<cv::Rect> faces;
    mFaceTracker->getObjects(faces);

    if (faces.empty()) {
        LogUtil::logI(TAG, {"trackFaceNative: no face detected"});
    } else {
        LogUtil::logI(TAG, {"trackFaceNative: face detected"});
        //only support lift one face now
        dlib::rectangle dFaceRect;
        dFaceRect.set_left(faces[0].x);
        dFaceRect.set_top(faces[0].y);
        dFaceRect.set_right(faces[0].x + faces[0].width);
        dFaceRect.set_bottom(faces[0].y + faces[0].height);
        dlib::full_object_detection detection = mShapePredictor(bgrPixel, dFaceRect);
        Point lhsDst;
        Point lhsCtrl;
        Point rhsDst;
        Point rhsCtrl;
        float divider = 15;
        lhsDst.x = (float)(detection.part(0).x() + detection.part(8).x()) / 2;
        lhsDst.y = (float)(detection.part(0).y() + detection.part(8).y()) / 2;

        lhsCtrl.x = detection.part(0).x() - (float)(detection.part(27).x() - detection.part(0).x()) / divider;
        lhsCtrl.y = lhsDst.y + (lhsDst.x - lhsCtrl.x) * (float)(detection.part(8).x() - detection.part(0).x()) / (float)(detection.part(8).y() - detection.part(0).y());

        rhsDst.x = (float)(detection.part(8).x() + detection.part(16).x()) / 2;
        rhsDst.y = (float)(detection.part(8).y() + detection.part(16).y()) / 2;

        rhsCtrl.x = detection.part(16).x() + (float)(detection.part(16).x() - detection.part(27).x()) / divider;
        rhsCtrl.y = rhsDst.y + (rhsDst.x - rhsCtrl.x) * (float)(detection.part(16).x() - detection.part(8).x()) / (float)(detection.part(16).y() - detection.part(8).y());

        if (mFaceDetectCallback != nullptr) { mFaceDetectCallback(mCallback, &lhsDst, &lhsCtrl, &rhsDst, &rhsCtrl); }
    }
}

void FaceDetector::setCallback(void* (*pCallbackStart)(void *argStart),
        void* (*pCallbackStop)(void *argStop),
        void*(*pCallbackFaceDetect)(void* arg0, void* arg1, void* arg2, void* arg3, void* arg4),
        void* callback) {
    if (pCallbackStart != nullptr) { mTrackStartCallback = pCallbackStart; }
    if (pCallbackStop != nullptr) { mTrackStopCallback = pCallbackStop; }
    if (pCallbackFaceDetect != nullptr) { mFaceDetectCallback = pCallbackFaceDetect; }
    mCallback = callback;
}




