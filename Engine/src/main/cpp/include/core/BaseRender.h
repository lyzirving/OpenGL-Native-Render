//
// Created by liuyuzhou on 2021/9/24.
//
#ifndef ENGINE_BASERENDER_H
#define ENGINE_BASERENDER_H

#include <jni.h>

#include "Common.h"
#include "EventQueue.h"
#include "WorkQueue.h"
#include "RenderEglBase.h"

#include "ScreenFilter.h"
#include "BaseFilterGroup.h"

class BaseRender {
public:
    BaseRender();
    ~BaseRender();

    virtual bool addBeautyFilter(const char* filterType, bool commit) = 0;
    virtual void adjust(const char* filterType, int progress) = 0;
    virtual void clearBeautyFilter() = 0;
    virtual void drawFrame() = 0;
    virtual void release(JNIEnv* env);

    void enqueueMessage(EventType what);
    bool initialized();
    void notifyEnvPrepare(JNIEnv* env, jobject listener);
    void notifyEnvRelease(JNIEnv* env, jobject listener);
    void render(JNIEnv* env);
    void setJavaListener(JNIEnv* env, jobject listener);
    void setSurfaceSize(GLint surfaceWidth, GLint surfaceHeight);
    void setNativeWindow(ANativeWindow* window);

protected:
    virtual void handleOtherMessage(JNIEnv* env, EventType what) = 0;
    virtual void handlePreDraw(JNIEnv* env) = 0;
    virtual void handlePostDraw(JNIEnv* env) = 0;
    virtual void handleRenderEnvPause(JNIEnv* env) = 0;
    virtual void handleRenderEnvResume(JNIEnv* env) = 0;
    virtual void handleRenderEnvDestroy(JNIEnv* env) = 0;
    virtual void handleSurfaceChange(JNIEnv* env) = 0;

    void renderEnvPause();
    bool renderEnvResume();
    void renderEnvDestroy();
    void runTaskPreDraw();
    void surfaceChange();

    RenderEglBase* mEglCore;
    jobject mJavaListener{nullptr};
    EventQueue* mEvtQueue{nullptr};
    WorkQueue* mWorkQueue{nullptr};
    render::Status mStatus = render::Status::STATUS_IDLE;

    GLint mSurfaceWidth{0};
    GLint mSurfaceHeight{0};

    ScreenFilter* mScreenFilter{nullptr};
    BaseFilterGroup* mBeautyFilterGroup{nullptr};
};
#endif //ENGINE_BASERENDER_H
