//
// Created by liuyuzhou on 2021/10/27.
//
#include "ShareRender.h"
#include <thread>

#define TAG "ShareRender"
#define MAX_RETRY_COUNT 10
#define JAVA_CLASS_SHARE_RENDER "com/render/engine/share/ShareRenderEngine"

static jlong nEnvCreate(JNIEnv *env, jclass clazz) {
    return reinterpret_cast<jlong>(new ShareRender);
}

static JNINativeMethod sJniMethods[] = {
        {
                "nCreate",           "()J",
                (void *) nEnvCreate
        }
};

bool ShareRender::registerSelf(JNIEnv *env) {
    int count = sizeof(sJniMethods) / sizeof(sJniMethods[0]);
    jclass javaClass = env->FindClass(JAVA_CLASS_SHARE_RENDER);
    if (!javaClass) {
        LogUtil::logE(TAG, {"registerSelf: failed to find class ", JAVA_CLASS_SHARE_RENDER});
        return false;
    }
    if (env->RegisterNatives(javaClass, sJniMethods, count) < 0) {
        LogUtil::logE(TAG, {"registerSelf: failed to register native methods ", JAVA_CLASS_SHARE_RENDER});
        return false;
    }
    return true;
}

void ShareRender::drawFrame() {}

void ShareRender::drawShare(GLuint inputShareTexture, int curDrawCount) {
    if (mScreenFilter != nullptr && mScreenFilter->initialized()) {
        //the input texture has been drawn once, so drawCount should be initialized as 1;
        GLuint lastTexture = inputShareTexture;
        if (mBeautyFilterGroup != nullptr && mBeautyFilterGroup->initialized()) {
            lastTexture = mBeautyFilterGroup->onDraw(lastTexture);
            curDrawCount += mBeautyFilterGroup->filterSize();
        }
        notifyShareEnvDraw(lastTexture, curDrawCount);
        bool isOdd = (curDrawCount % 2) != 0;
        mScreenFilter->flip(false, isOdd);
        mScreenFilter->onDraw(lastTexture);
    } else {
        LogUtil::logI(TAG, {"drawShare: filter is not valid"});
    }
}

void ShareRender::handleEnvPrepare(JNIEnv *env) {}

void ShareRender::handleRenderEnvPause(JNIEnv *env) {
    if (mBeautyFilterGroup != nullptr) { mBeautyFilterGroup->destroy(); }
    delete mBeautyFilterGroup;
    mBeautyFilterGroup = nullptr;
}

void ShareRender::handleRenderEnvDestroy(JNIEnv *env) {}

void ShareRender::handleOtherMessage(JNIEnv *env, const EventMessage &msg) {}

void ShareRender::handlePreDraw(JNIEnv *env) {}

void ShareRender::handlePostDraw(JNIEnv *env) {
    if (!mEglCore->swapBuffer()) {
        LogUtil::logI(TAG, {"handlePostDraw: failed to swap buffer"});
    }
}

void ShareRender::handleRenderEnvResume(JNIEnv *env) {}

void ShareRender::handleSurfaceChange(JNIEnv *env) {
    if (mBeautyFilterGroup != nullptr && !mBeautyFilterGroup->initialized()) { mBeautyFilterGroup->init(); }
}

void ShareRender::render(JNIEnv *env) {
    LogUtil::logI(TAG, {"render: enter"});
    mStatus = render::Status::STATUS_WAIT_CONTEXT;
    int retryCount = 0;
    for (;;) {
        EventMessage message = mEventQueue->dequeue();
        switch (message.what) {
            case EventType::EVENT_ADD_SHARE_CONTEXT: {
                LogUtil::logI(TAG, {"render: handle message add share context"});
                if (!mEglCore->valid()) {
                    if (!mEglCore->initEglEnv(mShareContext)) { goto quit; }
                    mStatus = render::Status::STATUS_PREPARED;
                    handleEnvPrepare(env);
                    notifyEnvPrepare(env, GET_LISTENER);
                    enqueueMessage(EventType::EVENT_SURFACE_CHANGE);
                } else {
                    LogUtil::logI(TAG, {"render: egl env is already initialized"});
                }
                break;
            }
            case EventType::EVENT_DRAW_SHARE_ENV: {
                bool valid = mEglCore != nullptr && mEglCore->valid();
                if (valid) {
                    mStatus = render::Status::STATUS_RUN;
                    runTaskPreDraw();
                    handlePreDraw(env);
                    drawShare(message.arg0, message.arg1 < 0 ? 0 : message.arg1);
                    handlePostDraw(env);
                } else {
                    LogUtil::logI(TAG, {"render: handle draw share env, env is not valid"});
                }
                break;
            }
            case EventType::EVENT_SURFACE_CHANGE: {
                LogUtil::logI(TAG, {"render: handle message surface change"});
                if (!mEglCore->valid()) {
                    LogUtil::logI(TAG, {"render: egl not valid"});
                    if (retryCount > MAX_RETRY_COUNT) {
                        LogUtil::logI(TAG, {"render: retry count > 10, quit"});
                        goto quit;
                    }
                    retryCount++;
                    std::this_thread::sleep_for(std::chrono::seconds(1));
                    enqueueMessage(EventType::EVENT_SURFACE_CHANGE);
                } else if (mSurfaceWidth == 0 || mSurfaceHeight == 0) {
                    LogUtil::logI(TAG, {"render: surface size not valid"});
                    mSurfaceWidth = mEglCore->getWindowWidth();
                    mSurfaceHeight = mEglCore->getWindowHeight();
                    if (retryCount > MAX_RETRY_COUNT) {
                        LogUtil::logI(TAG, {"render: retry count > 10, quit"});
                        goto quit;
                    }
                    retryCount++;
                    std::this_thread::sleep_for(std::chrono::seconds(1));
                    enqueueMessage(EventType::EVENT_SURFACE_CHANGE);
                } else {
                    retryCount = 0;
                    mStatus = render::Status::STATUS_RUN;
                    surfaceChange();
                    handleSurfaceChange(env);
                }
                break;
            }
            case EventType::EVENT_PAUSE: {
                LogUtil::logI(TAG, {"render: handle message pause"});
                mStatus = render::Status::STATUS_PAUSE;
                renderEnvPause();
                handleRenderEnvPause(env);
                break;
            }
            case EventType::EVENT_QUIT: {
                LogUtil::logI(TAG, {"render: handle message quit"});
                mStatus = render::Status::STATUS_DESTROY;
                renderEnvDestroy();
                handleRenderEnvDestroy(env);
                goto quit;
            }
            default: {
                LogUtil::logI(TAG, {"render: handle default msg"});
                handleOtherMessage(env, message);
                break;
            }
        }
    }
    quit:
    LogUtil::logI(TAG, {"render: quit render-loop"});
    notifyEnvRelease(env, GET_LISTENER);
    release(env);
    delete this;
}

void ShareRender::trackFace(bool start) {}


