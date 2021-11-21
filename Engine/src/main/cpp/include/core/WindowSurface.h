//
// Created by lyzirving on 2021/11/20.
//
#ifndef ENGINE_SURFACEWINDOW_H
#define ENGINE_SURFACEWINDOW_H

#include <EGL/egl.h>
#include <EGL/eglext.h>
#include <android/native_window_jni.h>

class WindowSurface {
private:
    EGLint mWidth;
    EGLint mHeight;
    EGLSurface mEglWindowSurface{EGL_NO_SURFACE};
public:

    WindowSurface(EGLDisplay display, EGLConfig config, ANativeWindow *window);

    bool checkValid();

    int getWidth();

    int getHeight();

    bool isPrepare();

    void makeCurrent(EGLDisplay, EGLContext context);

    void release(EGLDisplay display);

    void swapBuffer(EGLDisplay display);
};

#endif //ENGINE_SURFACEWINDOW_H
