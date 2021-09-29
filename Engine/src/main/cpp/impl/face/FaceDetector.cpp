//
// Created by liuyuzhou on 2021/9/28.
//
#include <unistd.h>
#include <sys/stat.h>
#include <cstdio>

#include "png.h"

#include "FaceDetector.h"
#include "JniUtil.h"
#include "Common.h"
#include "LogUtil.h"

#define TAG "FaceDetector"
#define CLASSIFIER_PATH "/storage/emulated/0/Android/data/com.render.demo/files/Documents/lbpcascade_frontalface_improved.xml"

const char* rootPath = reinterpret_cast<const char *>("/storage/emulated/0/testImage");

FaceDetector::FaceDetector() {
    mMessageQueue = new ObjectQueue<EventMessage>;
    pthread_mutex_init(&mQuitMutexLock, nullptr);
    pthread_cond_init(&mQuitCondLock, nullptr);
}

FaceDetector::~FaceDetector() {
    delete mMessageQueue;
    delete mImgQueue;
    mImgQueue = nullptr;
    mMessageQueue = nullptr;
    mStatus = render::Status::STATUS_IDLE;
    pthread_mutex_destroy(&mQuitMutexLock);
    pthread_cond_destroy(&mQuitCondLock);
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

void FaceDetector::buildTracker() {
    releaseTracker();
    cv::Ptr<cv::CascadeClassifier> mainClassifier = cv::makePtr<cv::CascadeClassifier>(CLASSIFIER_PATH);
    cv::Ptr<CascadeDetectorAdapter> mainDetector = cv::makePtr<CascadeDetectorAdapter>(mainClassifier);

    cv::Ptr<cv::CascadeClassifier> subClassifier = cv::makePtr<cv::CascadeClassifier>(CLASSIFIER_PATH);
    cv::Ptr<CascadeDetectorAdapter> subDetector = cv::makePtr<CascadeDetectorAdapter>(subClassifier);

    cv::DetectionBasedTracker::Parameters params;
    mFaceTracker = cv::makePtr<cv::DetectionBasedTracker>(mainDetector, subDetector, params);
    bool res = mFaceTracker->run();
    LogUtil::logI(TAG, {"buildTracker: result = ", (res ? "true" : "false")});
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

bool FaceDetector::isRunning() {
    return mStatus == render::Status::STATUS_RUN;
}

void FaceDetector::loop(JNIEnv *env) {
    mStatus = render::Status::STATUS_PREPARING;
    buildTracker();
    LogUtil::logI(TAG, {"loop: enter"});
    mStatus = render::Status::STATUS_RUN;
    for (;;) {
        EventMessage msg = mMessageQueue->dequeue();
        switch (msg.what) {
            case EventType::EVENT_WRITE_PNG: {
                LogUtil::logI(TAG, {"loop: handle write png"});
                std::shared_ptr<Image> img = mImgQueue->dequeue();
                if (img->data != nullptr) {
                    writePngImage(img->data, img->width, img->height, img->channel);
                }
                break;
            }
            case EventType::EVENT_FACE_TRACK: {
                std::shared_ptr<Image> img = mImgQueue->dequeue();
                if (img->data != nullptr) {
                    trackFace(img->data, img->width, img->height, img->channel);
                }
                break;
            }
            case EventType::EVENT_QUIT: {
                LogUtil::logI(TAG, {"loop: handle quit"});
                mStatus = render::Status::STATUS_DESTROY;
                goto quit;
            }
            default: {
                break;
            }
        }
    }
    quit:
    releaseTracker();
    pthread_mutex_lock(&mQuitMutexLock);
    pthread_cond_signal(&mQuitCondLock);
    pthread_mutex_unlock(&mQuitMutexLock);
    LogUtil::logI(TAG, {"loop: quit completely"});
}

void FaceDetector::prepare(JNIEnv* env) {
    render::getJvm(env);
    if (isRunning()) {
        LogUtil::logI(TAG, {"prepare: still run status"});
    } else {
        LogUtil::logI(TAG, {"prepare: create thread"});
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

void FaceDetector::quit() {
    if (isRunning()) {
        LogUtil::logI(TAG, {"quit"});
        mStatus = render::Status::STATUS_DESTROY;
        mMessageQueue->enqueue(EventMessage(EventType::EVENT_QUIT));
    }
}

void FaceDetector::quitAndWait() {
    if (isRunning()) {
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

void FaceDetector::trackFace(unsigned char *data, int width, int height, int channel) {
    cv::Mat gray;
    cv::Mat src(height, width, CV_8UC4, data);
    cv::cvtColor(src, gray, cv::COLOR_RGBA2GRAY);
    cv::equalizeHist(gray, gray);
    mFaceTracker->process(gray);
    std::vector<cv::Rect> faces;
    mFaceTracker->getObjects(faces);
    LogUtil::logI(TAG, {"trackFace: result = ", std::to_string(faces.size())});
}
