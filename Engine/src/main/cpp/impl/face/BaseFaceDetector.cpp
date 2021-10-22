//
// Created by liuyuzhou on 2021/10/13.
//
#include <unistd.h>
#include <sys/stat.h>
#include <cstdio>

#include "BaseFaceDetector.h"
#include "png.h"
#include "JniUtil.h"

#define TAG "BaseFaceDetector"

#define FACE_LANDMARK_PATH "/storage/emulated/0/Android/data/com.render.demo/files/Documents/shape_predictor_68_face_landmarks.dat"
const char* copyRootPath = reinterpret_cast<const char *>("/storage/emulated/0/testImage");

BaseFaceDetector::BaseFaceDetector() {
    mMessageQueue = new ObjectQueue<EventMessage>;
    pthread_mutex_init(&mQuitMutexLock, nullptr);
    pthread_cond_init(&mQuitCondLock, nullptr);

    pthread_mutex_init(&mStatusLock, nullptr);
}

BaseFaceDetector::~BaseFaceDetector() {
    delete mMessageQueue;
    delete mImgQueue;
    mImgQueue = nullptr;
    mMessageQueue = nullptr;
    mStatus = render::Status::STATUS_IDLE;

    pthread_mutex_destroy(&mQuitMutexLock);
    pthread_cond_destroy(&mQuitCondLock);

    pthread_mutex_destroy(&mStatusLock);
}

void *detectLoop(void *args) {
    auto *detector = static_cast<BaseFaceDetector *>(args);
    JNIEnv *env = nullptr;
    if (!JniUtil::self()->attachJvm(&env)) {
        LogUtil::logI(TAG, {"renderLoop: failed to attach thread to jvm"});
        return nullptr;
    }
    detector->loop(env);
    JniUtil::self()->detachThread();
    return nullptr;
}

void BaseFaceDetector::buildEnv() {
    releaseTracker();
    buildTracker();
    buildFacePredictor();
    handleFinishBuildEnv();
}

void BaseFaceDetector::buildFacePredictor() {
    dlib::deserialize(FACE_LANDMARK_PATH) >> mShapePredictor;
    LogUtil::logI(TAG, {"buildFacePredictor"});
}

void BaseFaceDetector::changeStatus(render::Status newState) {
    if (newState >= render::Status::STATUS_IDLE && newState <= render::Status::STATUS_DESTROY) {
        pthread_mutex_lock(&mStatusLock);
        mStatus = newState;
        pthread_mutex_unlock(&mStatusLock);
    }
}

void BaseFaceDetector::copyImageLocal(const unsigned char *data, int width, int height, int channel) {
    if (data == nullptr) {
        LogUtil::logI(TAG, {"copyImageLocal: input is null"});
        return;
    }
    if (access(copyRootPath, 0) == -1) { mkdir(copyRootPath, 0666); }
    char imgPath[256] = {0};
    char pngTitle[256] = {0};
    sprintf(imgPath, "%s/IMG_%lld.%s", copyRootPath, render::getCurrentTimeMs(), "png");
    sprintf(pngTitle, "%s/IMG_%lld", copyRootPath, render::getCurrentTimeMs());

    FILE *dst = fopen(imgPath, "wb");
    png_structp pngStruct = nullptr;
    png_infop pngInfo = nullptr;
    png_bytep row = nullptr;
    png_text title_text;
    if (dst == nullptr) {
        LogUtil::logI(TAG, {"copyImageLocal: failed to open file ", imgPath});
        goto end;
    }
    pngStruct = png_create_write_struct(PNG_LIBPNG_VER_STRING, nullptr, nullptr, nullptr);
    if (pngStruct == nullptr) {
        LogUtil::logI(TAG, {"copyImageLocal: failed to create png struct ", imgPath});
        goto end;
    }
    pngInfo = png_create_info_struct(pngStruct);
    if (pngInfo == nullptr) {
        LogUtil::logI(TAG, {"copyImageLocal: failed to create png info ", imgPath});
        goto end;
    }
    if(setjmp(png_jmpbuf(pngStruct))) {
        LogUtil::logI(TAG, {"copyImageLocal: error during creation"});
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

void BaseFaceDetector::enqueueImg(unsigned char *data, int width, int height, int channel, EventType type) {
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

void BaseFaceDetector::execute(bool start) {
    if (start) {
        mMessageQueue->enqueue(EventMessage(EventType::EVENT_CHANGE_STATUS, static_cast<int>(render::Status::STATUS_RUN)));
    } else {
        mMessageQueue->enqueue(EventMessage(EventType::EVENT_CHANGE_STATUS, static_cast<int>(render::Status::STATUS_PAUSE)));
    }
}

render::Status BaseFaceDetector::getStatus() {
    render::Status result;
    pthread_mutex_lock(&mStatusLock);
    result = mStatus;
    pthread_mutex_unlock(&mStatusLock);
    return result;
}

void BaseFaceDetector::handleFinishBuildEnv() {}

bool BaseFaceDetector::isRunning() {
    return getStatus() == render::Status::STATUS_RUN;
}

void BaseFaceDetector::loop(JNIEnv *env) {
    changeStatus(render::Status::STATUS_PREPARING);
    buildEnv();
    changeStatus(render::Status::STATUS_PREPARED);
    LogUtil::logI(TAG, {"loop: prepared"});
    for (;;) {
        EventMessage msg = mMessageQueue->dequeue();
        render::Status curState = getStatus();
        if (msg.what != EventType::EVENT_CHANGE_STATUS && msg.what != EventType::EVENT_QUIT
            && curState != render::Status::STATUS_RUN) {
            LogUtil::logI(TAG, {"loop: state is not running, ignore msg ", std::to_string(static_cast<int>(msg.what))});
            mImgQueue->clear();
            continue;
        }
        switch (msg.what) {
            case EventType::EVENT_WRITE_PNG: {
                std::shared_ptr<Image> img = mImgQueue->dequeue();
                if (img->data != nullptr) {
                    copyImageLocal(img->data, img->width, img->height, img->channel);
                }
                break;
            }
            case EventType::EVENT_CHANGE_STATUS: {
                LogUtil::logI(TAG, {"loop: handle change status, new state = ", std::to_string(msg.arg0)});
                auto newState = static_cast<render::Status>(msg.arg0);
                changeStatus(newState);
                if (newState == render::Status::STATUS_RUN) {
                    notifyRunTrack(env);
                } else if (newState == render::Status::STATUS_PAUSE) {
                    notifyPauseTrack(env);
                }
                break;
            }
            case EventType::EVENT_QUIT: {
                LogUtil::logI(TAG, {"loop: handle quit"});
                changeStatus(render::Status::STATUS_DESTROY);
                notifyQuitTrack(env);
                goto quit;
            }
            default: {
                LogUtil::logI(TAG, {"loop: handle default"});
                handleOtherMessage(env, msg);
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

void BaseFaceDetector::notifyPauseTrack(JNIEnv *env) {
    if (mCallbackPauseTrack != nullptr) { mCallbackPauseTrack(env, mFaceListener); }
}

void BaseFaceDetector::notifyRunTrack(JNIEnv *env) {}

void BaseFaceDetector::notifyQuitTrack(JNIEnv *env) {}

void BaseFaceDetector::prepare(JNIEnv* env) {
    if (isRunning()) {
        LogUtil::logI(TAG, {"prepare: still run status"});
    } else {
        LogUtil::logI(TAG, {"prepare: create thread"});
        if (mImgQueue != nullptr) { mImgQueue->notify(); }
        delete mImgQueue;
        mImgQueue = new PointerQueue<Image>;
        pthread_t thread;
        pthread_create(&thread, nullptr, detectLoop, this);
    }
}

void BaseFaceDetector::quitAndWait() {
    render::Status status = getStatus();
    if (status < render::Status::STATUS_DESTROY) {
        pthread_mutex_lock(&mQuitMutexLock);
        mMessageQueue->enqueue(EventMessage(EventType::EVENT_QUIT));
        LogUtil::logI(TAG, {"quitAndWait: wait"});
        pthread_cond_wait(&mQuitCondLock, &mQuitMutexLock);
        LogUtil::logI(TAG, {"quitAndWait: resume"});
    }
}
