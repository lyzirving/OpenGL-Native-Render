//
// Created by lyzirving on 2021/11/20.
//
#include "WindowSurface.h"
#include "Log.h"

#define TAG "WindowSurface"

WindowSurface::WindowSurface(EGLDisplay display, EGLConfig config, ANativeWindow *window) {
    mEglWindowSurface = eglCreateWindowSurface(display, config, window, nullptr);
    EGLint error = eglGetError();
    if (mEglWindowSurface == EGL_NO_SURFACE || error != EGL_SUCCESS) {
        LOG_E("%s_constructor: failed to create window surface, error = %d", TAG, error);
        mEglWindowSurface = EGL_NO_SURFACE;
    } else {
        eglQuerySurface(display, mEglWindowSurface, EGL_WIDTH, &mWidth);
        eglQuerySurface(display, mEglWindowSurface, EGL_HEIGHT, &mHeight);
        LOG_I("%s_constructor: succeed to create window surface, width = %d, height = %d", TAG, mWidth, mHeight);
    }
}

bool WindowSurface::checkValid() {
    return mEglWindowSurface != EGL_NO_SURFACE;
}

int WindowSurface::getWidth() { return mWidth; }

int WindowSurface::getHeight() { return mHeight; }

bool WindowSurface::isPrepare() { return mEglWindowSurface != EGL_NO_SURFACE; }

void WindowSurface::makeCurrent(EGLDisplay display, EGLContext context) {
    if (isPrepare()) {
        if (!eglMakeCurrent(display, mEglWindowSurface, mEglWindowSurface, context)) {
            LOG_E("%s_makeCurrent: failed to make current, reason = %d", TAG, eglGetError());
        }
    } else {
        LOG_E("%s_makeCurrent: not ready", TAG);
    }
}

void WindowSurface::release(EGLDisplay display) {
    if (isPrepare()) {
        eglDestroySurface(display, mEglWindowSurface);
        mEglWindowSurface = EGL_NO_SURFACE;
    } else {
        LOG_E("%s_release: not ready", TAG);
    }
}

void WindowSurface::swapBuffer(EGLDisplay display) {
    if (isPrepare()) {
        if (!eglSwapBuffers(display, mEglWindowSurface)) {
            LOG_E("%s_swapBuffer: failed to swap buffer, reason = %d", TAG, eglGetError());
        }
    } else {
        LOG_E("%s_swapBuffer: not ready", TAG);
    }
}

