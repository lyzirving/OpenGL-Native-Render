//
// Created by liuyuzhou on 2021/9/24.
//
#ifndef ENGINE_BASERENDER_H
#define ENGINE_BASERENDER_H

#include <jni.h>

#include "Common.h"
#include "ValidPtr.h"
#include "EventMessage.h"
#include "WorkTask.h"
#include "RenderEglBase.h"
#include "CustomQueue.h"

#include "ScreenFilter.h"
#include "BeautifyFaceFilter.h"
#include "BaseFilterGroup.h"

#define GET_LISTENER (mListener != nullptr ? mListener->get() : nullptr)

class BaseRender {
public:
    static bool registerSelf(JNIEnv *env);

    BaseRender();
    virtual ~BaseRender();

    virtual void adjustProperty(const char *filterType, const char *property, int progress);
    virtual void drawFrame() = 0;
    virtual void drawShare(GLuint inputShareTexture, int curDrawCount);
    virtual GLuint getContentTexture();
    virtual void trackFace(bool start) = 0;
    virtual void release(JNIEnv* env);
    virtual void render(JNIEnv* env);

    void adjust(const char* filterType, int progress);
    bool addBeautyFilter(const char* filterType, bool commit);
    void bindShareEnv(long sharePtr);
    void clearBeautyFilter();
    void beautifyFace(bool start);
    void enqueueMessage(EventType what);
    void enqueueMessage(const EventMessage& msg);
    /**
     * get the egl render context
     * notice this method must be called in render thread
     * @return egl context pointer that is cast to type long
     */
    long getEglContext();
    bool initialized();
    void notifyEnvPrepare(JNIEnv* env, jobject listener);
    void notifyEnvResume(JNIEnv* env, jobject listener);
    void notifyEnvRelease(JNIEnv* env, jobject listener);
    bool surfacePrepare();
    void setJavaListener(JNIEnv* env, jobject listener);
    void setSurfaceSize(GLint surfaceWidth, GLint surfaceHeight);
    void setNativeWindow(ANativeWindow* window);
    void setShareEglContext(long context);

protected:
    virtual void handleEnvPrepare(JNIEnv* env) = 0;
    virtual void handleOtherMessage(JNIEnv* env, const EventMessage& msg) = 0;
    virtual void handlePreDraw(JNIEnv* env) = 0;
    virtual void handlePostDraw(JNIEnv* env) = 0;
    virtual void handleRenderEnvPause(JNIEnv* env) = 0;
    virtual void handleRenderEnvResume(JNIEnv* env) = 0;
    virtual void handleRenderEnvDestroy(JNIEnv* env) = 0;
    virtual void handleSurfaceChange(JNIEnv* env) = 0;
    virtual void notifyShareEnvDraw(GLuint textureId, int curDrawCount);

    void renderEnvPause();
    bool renderEnvResume();
    void renderEnvDestroy();
    void runTaskPreDraw();
    void surfaceChange();

    RenderEglBase* mEglCore;
    ValidPtr<_jobject>* mListener{nullptr};
    ObjectQueue<EventMessage>* mEventQueue{nullptr};
    PointerQueue<WorkTask>* mWorkQueue{nullptr};
    render::Status mStatus = render::Status::STATUS_IDLE;

    GLint mSurfaceWidth{0};
    GLint mSurfaceHeight{0};

    ScreenFilter* mScreenFilter{nullptr};
    BeautifyFaceFilter* mBeautifyFaceFilter{nullptr};
    BaseFilterGroup* mBeautyFilterGroup{nullptr};

    std::vector<BaseRender*>* mShareEnv{nullptr};
    EGLContext mShareContext{EGL_NO_CONTEXT};
};
#endif //ENGINE_BASERENDER_H
