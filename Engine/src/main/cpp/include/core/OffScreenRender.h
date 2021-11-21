//
// Created by lyzirving on 2021/11/19.
//
#ifndef ENGINE_OFFSCREENRENDER_H
#define ENGINE_OFFSCREENRENDER_H

#include <android/native_window_jni.h>
#include <android/surface_texture_jni.h>
#include <EGL/egl.h>
#include <EGL/eglext.h>
#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>
#include <GLES3/gl3.h>
#include <GLES3/gl3ext.h>

#include "WindowSurface.h"
#include "Common.h"
#include "CustomQueue.h"
#include "EventMessage.h"

#include "OesTexFilter.h"
#include "DefaultFilter.h"

class OffScreenRender {
private:

    EGLContext mEglContext{EGL_NO_CONTEXT};
    EGLDisplay mEglDisplay{EGL_NO_DISPLAY};
    EGLConfig mEglConfig{EGL_NO_CONFIG_KHR};

    GLuint mOffScreenFrameBuffer{0};
    GLuint mOffScreenTexture{0};
    GLuint mOesTexture{0};

    ANativeWindow* mOffScreenWindow{nullptr};
    WindowSurface *mOffScreenWindowSurface{nullptr};

    ANativeWindow *mClientWindow{nullptr};
    WindowSurface *mClientSurface{nullptr};
    ASurfaceTexture *mSurfaceTexture{nullptr};

    std::unique_ptr<_jobject> mJavaListener;
    ObjectQueue<EventMessage> *mEventQueue{nullptr};

    OesTexFilter* mOesTexFilter{nullptr};
    DefaultFilter* mDefaultFilter{nullptr};

    render::Status mStatus = render::Status::STATUS_IDLE;
    bool mLoopThreadCreate{false};
    float mTextureMatrix[16];

protected:
    void handleDraw(JNIEnv* env);

    void handlePreDraw(JNIEnv* env);

    void notifyEnvPrepare(JNIEnv *env, jobject listener, GLuint textureId);

    void notifyEnvRelease(JNIEnv *env, jobject listener);

public:
    static bool registerSelf(JNIEnv *env);

    OffScreenRender();

    ~OffScreenRender();

    bool checkLoopThread();

    void createOesTexture();

    void envDestroy();

    void envPause();

    bool envResume(JNIEnv* env);

    void enqueueMessage(EventType what);

    bool isPrepared();

    void loop(JNIEnv *env);

    bool prepareEgl();

    void prepareOffScreenFrameBuffer();

    void releaseEgl();

    void setJavaListener(JNIEnv *env, jobject listener);

    void setSurfaceTexture(JNIEnv *env, jobject texture);

    void setClientWindow(ANativeWindow *window);

    void setOffScreenWindow(ANativeWindow *window);

    void workBeforeKill(JNIEnv *env);

    void updateTexImage();
};

#endif //ENGINE_OFFSCREENRENDER_H
