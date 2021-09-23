//
// Created by liuyuzhou on 2021/9/7.
//
#ifndef ENGINE_RENDEREGLBASE_H
#define ENGINE_RENDEREGLBASE_H

#include <EGL/egl.h>
#include <android/native_window_jni.h>
#include <GLES2/gl2.h>

class RenderEglBase {
public:
    RenderEglBase();
    ~RenderEglBase();

    bool initEglEnv();
    bool makeCurrent();
    void release();
    void setNativeWindow(ANativeWindow* window);
    bool swapBuffer();
    bool valid();

private:
    ANativeWindow* mNativeWindow = nullptr;
    EGLConfig mEglConfig = nullptr;
    EGLDisplay mEglDisplay{EGL_NO_DISPLAY};
    EGLSurface mEglSurface{EGL_NO_SURFACE};
    EGLContext mEglContext{EGL_NO_CONTEXT};

    GLint mVersion{0};
};


#endif //ENGINE_RENDEREGLBASE_H
