//
// Created by liuyuzhou on 2021/9/7.
//
#include "RenderEglBase.h"
#include "LogUtil.h"

#define TAG "RenderEglBase"

RenderEglBase::RenderEglBase() = default;

RenderEglBase::~RenderEglBase() = default;

bool RenderEglBase::initEglEnv() {
    LogUtil::logI(TAG, {"initEglEnv"});
    auto *version = new EGLint[2];
    EGLint numConfig;
    EGLConfig *supportConfigs = nullptr;
    EGLint eglFormat;
    int i = 0;
    const EGLint glAttributes[] = {
            EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
            EGL_BLUE_SIZE, 8,
            EGL_GREEN_SIZE, 8,
            EGL_RED_SIZE, 8,
            EGL_ALPHA_SIZE, 8,
            /*EGL_DEPTH_SIZE, 8,
            EGL_STENCIL_SIZE, 8,
            EGL_RENDERABLE_TYPE, EGL_OPENGL_ES2_BIT,*/
            EGL_NONE//EGL_NONE means the process of match-up ends
    };
    const EGLint glContextAttribute[] = {
            EGL_CONTEXT_CLIENT_VERSION, 2,
            EGL_NONE};
    mEglDisplay = eglGetDisplay(EGL_DEFAULT_DISPLAY);
    if (mEglDisplay == EGL_NO_DISPLAY) {
        LogUtil::logI(TAG, {"initRenderEnv: failed to get display"});
        goto fail;
    }
    if (!eglInitialize(mEglDisplay, &version[0], &version[1])) {
        LogUtil::logI(TAG, {"initRenderEnv: failed to initialize display"});
        goto fail;
    }
    //get the the number of supported config
    if (!eglChooseConfig(mEglDisplay, glAttributes, nullptr, 0, &numConfig)) {
        LogUtil::logI(TAG, {"initRenderEnv: failed to get the number of config"});
        goto fail;
    }
    supportConfigs = new EGLConfig[numConfig];
    if (!eglChooseConfig(mEglDisplay, glAttributes, supportConfigs, numConfig, &numConfig)) {
        LogUtil::logI(TAG, {"initRenderEnv: failed to get configs"});
        goto fail;
    }
    for (; i < numConfig; i++) {
        auto &cfg = supportConfigs[i];
        EGLint r, g, b, alpha;
        if (eglGetConfigAttrib(mEglDisplay, cfg, EGL_RED_SIZE, &r) &&
            eglGetConfigAttrib(mEglDisplay, cfg, EGL_GREEN_SIZE, &g) &&
            eglGetConfigAttrib(mEglDisplay, cfg, EGL_BLUE_SIZE, &b) &&
            eglGetConfigAttrib(mEglDisplay, cfg, EGL_ALPHA_SIZE, &alpha) &&
            r == 8 && g == 8 && b == 8 && alpha == 8) {
            mEglConfig = supportConfigs[i];
            break;
        }
    }
    if (i == numConfig || mEglConfig == nullptr) {
        LogUtil::logI(TAG, {"initRenderEnv: failed to find target config"});
        goto fail;
    }
    eglGetConfigAttrib(mEglDisplay, mEglConfig, EGL_NATIVE_VISUAL_ID, &eglFormat);
    mEglSurface = eglCreateWindowSurface(mEglDisplay, mEglConfig, mNativeWindow, nullptr);
    if (mEglSurface == EGL_NO_SURFACE) {
        LogUtil::logI(TAG, {"initRenderEnv: failed to create window surface"});
        goto fail;
    }
    mEglContext = eglCreateContext(mEglDisplay, mEglConfig, nullptr, glContextAttribute);
    if (mEglContext == EGL_NO_CONTEXT) {
        LogUtil::logI(TAG, {"initRenderEnv: failed to create egl context"});
        goto fail;
    }
    if (!makeCurrent()) {
        LogUtil::logI(TAG, {"initRenderEnv: failed to make env current"});
        goto fail;
    }

    eglQueryContext(mEglDisplay, mEglContext, EGL_CONTEXT_CLIENT_VERSION, &mVersion);
    LogUtil::logI(TAG, {"initRenderEnv: version = ", std::to_string(mVersion)});
    return true;
    fail:
    release();
    return false;
}

bool RenderEglBase::makeCurrent() {
    return eglMakeCurrent(mEglDisplay, mEglSurface, mEglSurface, mEglContext) != EGL_FALSE;
}

void RenderEglBase::release() {
    LogUtil::logI(TAG, {"release"});
    if (mNativeWindow != nullptr) { ANativeWindow_release(mNativeWindow); }
    if (mEglContext != EGL_NO_CONTEXT) { eglDestroyContext(mEglDisplay, mEglContext); }
    if (mEglSurface != EGL_NO_SURFACE) { eglDestroySurface(mEglDisplay, mEglSurface); }
    if (mEglDisplay != EGL_NO_DISPLAY) {
        eglMakeCurrent(mEglDisplay, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
        eglTerminate(mEglDisplay);
    }
    mEglContext = EGL_NO_CONTEXT;
    mEglSurface = EGL_NO_SURFACE;
    mEglDisplay = EGL_NO_DISPLAY;
    mNativeWindow = nullptr;
    mEglConfig = nullptr;
}

void RenderEglBase::setNativeWindow(ANativeWindow* window) {
    mNativeWindow = window;
}

bool RenderEglBase::swapBuffer() {
    return eglSwapBuffers(mEglDisplay, mEglSurface) != EGL_FALSE;
}

