//
// Created by liuyuzhou on 2021/9/28.
//
#include <unistd.h>
#include <sys/stat.h>
#include <stdio.h>

#include "FaceDetector.h"
#include "JniUtil.h"
#include "Common.h"
#include "LogUtil.h"

#define TAG "FaceDetector"

const char* rootPath = reinterpret_cast<const char *>("/storage/emulated/0/testImage");

FaceDetector::FaceDetector() {
    mImageQueue = new ObjectQueue<Image>;
    mMessageQueue = new ObjectQueue<EventMessage>;
}

FaceDetector::~FaceDetector() {
    delete mImageQueue;
    delete mMessageQueue;
    mImageQueue = nullptr;
    mMessageQueue = nullptr;
    mStatus = render::Status::STATUS_IDLE;
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
    if (data) {
        Image image{width, height, channel};
        memcpy(image.data, data, width * height * channel);
        mImageQueue->enqueue(image);
    }
}

void FaceDetector::loop(JNIEnv *env) {
    LogUtil::logI(TAG, {"loop: enter"});
    mStatus = render::Status::STATUS_RUN;
    for (;;) {
        EventMessage msg = mMessageQueue->dequeue();
        switch (msg.what) {
            case EventType::EVENT_DOWN_LOAD: {
                LogUtil::logI(TAG, {"loop: handle down load"});
                Image image = mImageQueue->dequeueNotWait();
                if (image.data != nullptr) {
                    writeImageToFile(image.data, image.width * image.height * image.channel);
                    delete image.data;
                }
                break;
            }
            case EventType::EVENT_QUIT: {
                LogUtil::logI(TAG, {"loop: handle quit"});
                goto quit;
            }
            default: {
                break;
            }
        }
    }
    quit:
    LogUtil::logI(TAG, {"loop: quit"});
    mStatus = render::Status::STATUS_DESTROY;
    mImageQueue->clear();
}

void FaceDetector::prepare(JNIEnv* env) {
    render::getJvm(env);
    if (mStatus == render::Status::STATUS_RUN) {
        LogUtil::logI(TAG, {"prepare: still run status"});
    } else {
        LogUtil::logI(TAG, {"prepare: create thread"});
        pthread_t thread;
        pthread_create(&thread, nullptr, processLoop, this);
    }
}

void FaceDetector::writeImageToFile(const unsigned char *data, int size) {
    if (data == nullptr) {
        LogUtil::logI(TAG, {"writeImageToFile: input is null"});
        return;
    }
    if (access(rootPath, 0) == -1) { mkdir(rootPath, 0666); }
    char imgPath[256] = {0};
    sprintf(imgPath, "%s/IMG_%lld.%s", rootPath, render::getCurrentTimeMs(), "png");

    FILE *dst = fopen(imgPath, "wb");
    if (dst) {
        fwrite(data, size, 1, dst);
        fclose(dst);
        dst = nullptr;
    }
}

void FaceDetector::quit() {
    mMessageQueue->enqueue(EventMessage(EventType::EVENT_QUIT));
}
