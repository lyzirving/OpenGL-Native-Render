//
// Created by liuyuzhou on 2021/9/24.
//
#include "BaseRender.h"

#include "JniUtil.h"
#include "LogUtil.h"

#define TAG "BaseRender"

BaseRender::BaseRender() {
    mWorkQueue = new WorkQueue;
    mEvtQueue = new EventQueue;
    mEglCore = new RenderEglBase;
}

BaseRender::~BaseRender() = default;;

void BaseRender::enqueueMessage(EventType what) {
    mEvtQueue->enqueueMessage(new EventMessage(what));
}

bool BaseRender::initialized() {
    return mStatus >= render::Status::STATUS_PREPARE && mStatus <= render::Status::STATUS_PAUSE;
}

void BaseRender::notifyEnvPrepare(JNIEnv *env, jobject listener) {
    if (listener != nullptr) {
        jclass listenerClass = env->GetObjectClass(listener);
        jmethodID methodId = env->GetMethodID(listenerClass, "onRenderEnvPrepare", "()V");
        env->CallVoidMethod(listener, methodId);
    }
}

void BaseRender::notifyEnvRelease(JNIEnv *env, jobject listener) {
    if (listener != nullptr) {
        jclass listenerClass = env->GetObjectClass(listener);
        jmethodID methodId = env->GetMethodID(listenerClass, "onRenderEnvRelease", "()V");
        env->CallVoidMethod(listener, methodId);
    }
}

void BaseRender::render(JNIEnv *env) {
    if (!mEglCore->initEglEnv()) { goto quit; }
    mStatus = render::Status::STATUS_PREPARE;
    notifyEnvPrepare(env, mJavaListener);
    for (;;) {
        EventMessage &&message = mEvtQueue->dequeueMessage();
        switch (message.what) {
            case EventType::EVENT_SURFACE_CHANGE: {
                LogUtil::logI(TAG, {"render: handle message surface change"});
                mStatus = render::Status::STATUS_RUN;
                surfaceChange();
                handleSurfaceChange(env);
                break;
            }
            case EventType::EVENT_PAUSE: {
                LogUtil::logI(TAG, {"render: handle message pause"});
                mStatus = render::Status::STATUS_PAUSE;
                renderEnvPause();
                handleRenderEnvPause(env);
                break;
            }
            case EventType::EVENT_RESUME: {
                LogUtil::logI(TAG, {"render: handle message resume"});
                mStatus = render::Status::STATUS_RUN;
                bool success = renderEnvResume();
                if (success) {
                    notifyEnvPrepare(env, mJavaListener);
                    handleRenderEnvResume(env);
                } else {
                    goto quit;
                }
                break;
            }
            case EventType::EVENT_DRAW: {
                bool valid = mEglCore != nullptr && mEglCore->valid();
                if (valid) {
                    mStatus = render::Status::STATUS_RUN;
                    runTaskPreDraw();
                    handlePreDraw(env);
                    drawFrame();
                    handlePostDraw(env);
                } else {
                    LogUtil::logI(TAG, {"render: handle message draw, env is not valid"});
                }
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
                handleOtherMessage(env, message.what);
                break;
            }
        }
    }
    quit:
    LogUtil::logI(TAG, {"render: quit render-loop"});
    notifyEnvRelease(env, mJavaListener);
    release(env);
    delete this;
}

void BaseRender::release(JNIEnv *env) {
    if (mJavaListener != nullptr) { env->DeleteGlobalRef(mJavaListener); }
    if (mEglCore != nullptr) { mEglCore->release(); }
    delete mEglCore;
    delete mEvtQueue;
    delete mWorkQueue;
    mJavaListener = nullptr;
    mWorkQueue = nullptr;
    mEglCore = nullptr;
    mEvtQueue = nullptr;
}

void BaseRender::renderEnvPause() {
    mEvtQueue->clear();
    mWorkQueue->clear();
    mEglCore->release();
}

bool BaseRender::renderEnvResume() {
    return mEglCore->initEglEnv();
}

void BaseRender::renderEnvDestroy() {
    mEvtQueue->clear();
    mWorkQueue->clear();
}

void BaseRender::runTaskPreDraw() {
    while (!mWorkQueue->empty()) {
        std::shared_ptr<WorkTask> task = std::make_shared<WorkTask>();
        if (mWorkQueue->dequeue(task)) { task->run(); }
    }
}

void BaseRender::setJavaListener(JNIEnv *env, jobject listener) {
    if (mJavaListener != nullptr) { env->DeleteGlobalRef(mJavaListener); }
    if (listener != nullptr) { mJavaListener = env->NewGlobalRef(listener); }
}

void BaseRender::setNativeWindow(ANativeWindow *window) {
    mEglCore->setNativeWindow(window);
}

void BaseRender::setSurfaceSize(GLint surfaceWidth, GLint surfaceHeight) {
    mSurfaceWidth = surfaceWidth;
    mSurfaceHeight = surfaceHeight;
}

void BaseRender::surfaceChange() {
    glViewport(0, 0, mSurfaceWidth, mSurfaceHeight);
    if (!mEglCore->swapBuffer()) { LogUtil::logI(TAG, {"surfaceChange: failed to swap buffer"}); }
}