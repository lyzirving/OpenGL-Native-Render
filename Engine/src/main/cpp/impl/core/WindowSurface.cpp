//
// Created by lyzirving on 2021/11/20.
//
#include "WindowSurface.h"
#include "Log.h"

#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>
#include <GLES3/gl3.h>
#include <GLES3/gl3ext.h>

#define TAG "WindowSurface"

WindowSurface::WindowSurface(EGLDisplay display, EGLConfig config, ANativeWindow *window) {
    mEglWindowSurface = eglCreateWindowSurface(display, config, window, nullptr);
    EGLint error = eglGetError();
    if (mEglWindowSurface == EGL_NO_SURFACE || error != EGL_SUCCESS) {
        LOG_E("%s_constructor: failed to create window surface, error = %d", TAG, error);
        mEglWindowSurface = EGL_NO_SURFACE;
        mEglConfig = EGL_NO_CONFIG_KHR;
    } else {
        eglQuerySurface(display, mEglWindowSurface, EGL_WIDTH, &mWidth);
        eglQuerySurface(display, mEglWindowSurface, EGL_HEIGHT, &mHeight);
        LOG_I("%s_constructor: succeed to create window surface, width = %d, height = %d", TAG, mWidth, mHeight);
    }
}

WindowSurface::WindowSurface(EGLDisplay display, EGLint width, EGLint height) {
    EGLint numConfig;
    const EGLint glAttributes[] = {
            EGL_SURFACE_TYPE, EGL_PBUFFER_BIT,
            EGL_BLUE_SIZE, 8,
            EGL_GREEN_SIZE, 8,
            EGL_RED_SIZE, 8,
            EGL_ALPHA_SIZE, 8,
            EGL_DEPTH_SIZE, 8,
            EGL_STENCIL_SIZE, 8,
            EGL_RENDERABLE_TYPE, EGL_OPENGL_ES3_BIT_KHR,
            EGL_NONE//EGL_NONE means the process of match-up ends
    };
    if (!eglChooseConfig(display, glAttributes, &mEglConfig, 1, &numConfig)) {
        LOG_E("%s_WindowSurface: off screen window surface, failed to choose config", TAG);
        mEglWindowSurface = EGL_NO_SURFACE;
        return;
    }
    EGLint pBufferAttribute[] = {EGL_WIDTH, width,
                                 EGL_HEIGHT, height,
                                 EGL_NONE};
    mEglWindowSurface = eglCreatePbufferSurface(display, mEglConfig, pBufferAttribute);
    EGLint res = eglGetError();
    if (mEglWindowSurface == EGL_NO_SURFACE || res != EGL_SUCCESS) {
        mEglWindowSurface = EGL_NO_SURFACE;
        mEglConfig = EGL_NO_CONFIG_KHR;
        LOG_E("%s_WindowSurface: off screen window surface, failed to create pbuffer surface, reason = %d", TAG, res);
    } else {
        LOG_I("%s_off screen window surface: succeed to create pbuffer surface", TAG);
        mOffScreen = true;
        mWidth = width;
        mHeight = height;
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
        mEglConfig = EGL_NO_CONFIG_KHR;
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

/**
 * used only for off screen window surface
 */
void WindowSurface::swapBuffer() {
    if (isPrepare()) {
        glFlush();
    } else {
        LOG_E("%s_swapBuffer: not ready", TAG);
    }
}

