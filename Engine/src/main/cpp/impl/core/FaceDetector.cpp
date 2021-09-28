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

const char* rootPath = reinterpret_cast<const char *>("/storage/emulated/0/testImage");

FaceDetector::FaceDetector() {
    mImgQueue = new PointerQueue<Image>;
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

void FaceDetector::copyAndEnqueueData(const unsigned char *data, int width, int height, int channel) {
    LogUtil::logI(TAG, {"copyAndEnqueueData"});
    if (data != nullptr) {
        std::shared_ptr<Image> img = std::make_shared<Image>();
        img->width = width;
        img->height = height;
        img->channel = channel;
        img->data = static_cast<unsigned char *>(malloc(width * height * channel));
        memcpy(img->data, data, width * height * channel);
        mImgQueue->enqueue(img);
        mStatus = render::Status::STATUS_PROCESSING;
        mMessageQueue->enqueue(EventMessage(EventType::EVENT_DOWN_LOAD));
    }
}

bool FaceDetector::isRunning() {
    return mStatus == render::Status::STATUS_RUN;
}

bool FaceDetector::isProcessing() {
    return mStatus == render::Status::STATUS_PROCESSING;
}

void FaceDetector::loop(JNIEnv *env) {
    LogUtil::logI(TAG, {"loop: enter"});
    for (;;) {
        mStatus = render::Status::STATUS_RUN;
        EventMessage msg = mMessageQueue->dequeue();
        switch (msg.what) {
            case EventType::EVENT_DOWN_LOAD: {
                LogUtil::logI(TAG, {"loop: handle down load"});
                mStatus = render::Status::STATUS_PROCESSING;
                std::shared_ptr<Image> img = mImgQueue->dequeue();
                if (img->data != nullptr) {
                    writeImageToFile(img->data, img->width, img->height, img->channel);
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
    pthread_mutex_lock(&mQuitMutexLock);
    pthread_cond_signal(&mQuitCondLock);
    pthread_mutex_unlock(&mQuitMutexLock);
    LogUtil::logI(TAG, {"loop: quit"});
    mImgQueue->clear();
}

void FaceDetector::prepare(JNIEnv* env) {
    render::getJvm(env);
    if (isRunning() || isProcessing()) {
        LogUtil::logI(TAG, {"prepare: still run status"});
    } else {
        LogUtil::logI(TAG, {"prepare: create thread"});
        pthread_t thread;
        pthread_create(&thread, nullptr, processLoop, this);
    }
}

void FaceDetector::writeImageToFile(const unsigned char *data, int width, int height, int channel) {
    if (data == nullptr) {
        LogUtil::logI(TAG, {"writeImageToFile: input is null"});
        return;
    }
    int tmp = width;
    width = height;
    height = tmp;
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
        LogUtil::logI(TAG, {"writeImageToFile: failed to open file ", imgPath});
        goto end;
    }
    pngStruct = png_create_write_struct(PNG_LIBPNG_VER_STRING, nullptr, nullptr, nullptr);
    if (pngStruct == nullptr) {
        LogUtil::logI(TAG, {"writeImageToFile: failed to create png struct ", imgPath});
        goto end;
    }
    pngInfo = png_create_info_struct(pngStruct);
    if (pngInfo == nullptr) {
        LogUtil::logI(TAG, {"writeImageToFile: failed to create png info ", imgPath});
        goto end;
    }
    if(setjmp(png_jmpbuf(pngStruct))) {
        LogUtil::logI(TAG, {"writeImageToFile: error during creation"});
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
            if (x == 0 || x == (width - 1) || y == 0 || y == (height - 1)) {
                row[x * 3 + 0] = 0x00;
                row[x * 3 + 1] = 0x00;
                row[x * 3 + 2] = 0x00;
                row[x * 3 + 3] = 0xff;
            } else {
                row[x * 3 + 0] = data[x * 3 + 0];
                row[x * 3 + 1] = data[x * 3 + 1];
                row[x * 3 + 2] = data[x * 3 + 2];
                row[x * 3 + 3] = data[x * 3 + 3];
            }
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
        mMessageQueue->enqueue(EventMessage(EventType::EVENT_QUIT));
    }
}

void FaceDetector::quitAndWait() {
    if (isRunning() || isProcessing()) {
        pthread_mutex_lock(&mQuitMutexLock);
        mMessageQueue->enqueue(EventMessage(EventType::EVENT_QUIT));
        LogUtil::logI(TAG, {"quitAndWait: wait"});
        pthread_cond_wait(&mQuitCondLock, &mQuitMutexLock);
        LogUtil::logI(TAG, {"quitAndWait: resume"});
    }
}
