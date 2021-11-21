//
// Created by lyzirving on 2021/11/19.
//
#include "OffScreenRender.h"
#include "JniUtil.h"
#include "Log.h"

#include "pthread.h"

#define TAG "OffScreenRender"
#define JAVA_CLASS_OFF_SCREEN_RENDER "com/render/engine/core/OffScreenRender"
#define JAVA_CLASS_SURFACE_TEXTURE "android/graphics/SurfaceTexture"

static void *nativeLoop(void *args) {
    auto *render = static_cast<OffScreenRender *>(args);
    JNIEnv *env{nullptr};
    if (!JniUtil::self()->attachJvm(&env)) {
        LOG_E("%s_nativeLoop: failed to attach thread to jvm", TAG);
        return nullptr;
    }
    render->loop(env);
    JniUtil::self()->detachThread();
    return nullptr;
}

static jlong nativeCreate(JNIEnv *env, jclass clazz) {
    auto *render = new OffScreenRender;
    return reinterpret_cast<jlong>(render);
}

static void nativePrepare(JNIEnv *env, jclass clazz, jlong ptr, jobject surface, jobject adapter) {
    auto *render = reinterpret_cast<OffScreenRender *>(ptr);
    LOG_I("%s_nativePrepare", TAG);
    ANativeWindow *offScreenWindow = ANativeWindow_fromSurface(env, surface);
    render->setOffScreenWindow(offScreenWindow);
    render->setJavaListener(env, adapter);
    if (render->checkLoopThread()) {
        render->enqueueMessage(EventType::EVENT_RESUME);
    } else {
        if (!render->isPrepared()) {
            pthread_t thread;
            pthread_create(&thread, nullptr, nativeLoop, render);
        }
    }
}

static void nativeSetClient(JNIEnv *env, jclass clazz, jlong ptr, jobject surface) {
    auto *render = reinterpret_cast<OffScreenRender *>(ptr);
    ANativeWindow *nativeWindow = ANativeWindow_fromSurface(env, surface);
    render->setClientWindow(nativeWindow);
    render->enqueueMessage(EventType::EVENT_SET_CLIENT);
}

static void nativePause(JNIEnv *env, jclass clazz, jlong ptr) {
    auto *render = reinterpret_cast<OffScreenRender *>(ptr);
    render->enqueueMessage(EventType::EVENT_PAUSE);
}

static void nativeRelease(JNIEnv *env, jclass clazz, jlong ptr) {
    auto *render = reinterpret_cast<OffScreenRender *>(ptr);
    render->enqueueMessage(EventType::EVENT_QUIT);
}

static void nativeSetSurfaceTexture(JNIEnv *env, jclass clazz, jlong ptr, jobject surfaceTexture) {
    auto *render = reinterpret_cast<OffScreenRender *>(ptr);
    render->setSurfaceTexture(env, surfaceTexture);
}

static void nativeRequestRender(JNIEnv *env, jclass clazz, jlong ptr) {
    auto *render = reinterpret_cast<OffScreenRender *>(ptr);
    render->enqueueMessage(EventType::EVENT_DRAW);
}

static JNINativeMethod sJniMethods[] = {
        {
                "nCreate",            "()J",
                (void *) nativeCreate
        },
        {
                "nPrepare",           "(JLandroid/view/Surface;Lcom/render/engine/core/RenderAdapter;)V",
                (void *) nativePrepare
        },
        {
                "nSetClient",         "(JLandroid/view/Surface;)V",
                (void *) nativeSetClient
        },
        {
                "nPause",             "(J)V",
                (void *) nativePause
        },
        {
                "nRelease",           "(J)V",
                (void *) nativeRelease
        },
        {
                "nSetSurfaceTexture", "(JLandroid/graphics/SurfaceTexture;)V",
                (void *) nativeSetSurfaceTexture
        },
        {
                "nRequestRender",     "(J)V",
                (void *) nativeRequestRender
        },
};

OffScreenRender::OffScreenRender() {
    mEventQueue = new ObjectQueue<EventMessage>;
    mOesTexFilter = new OesTexFilter;
    mDefaultFilter = new DefaultFilter;
    for (int i = 0; i < 16; ++i) { mTextureMatrix[i] = 0; }
}

OffScreenRender::~OffScreenRender() = default;

void OffScreenRender::createOesTexture() {
    if (mOesTexture != 0) { glDeleteTextures(1, &mOesTexture); }
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glGenTextures(1, &mOesTexture);
    glBindTexture(GL_TEXTURE_EXTERNAL_OES, mOesTexture);
    glTexParameteri(GL_TEXTURE_EXTERNAL_OES, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_EXTERNAL_OES, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_EXTERNAL_OES, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_EXTERNAL_OES, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glBindTexture(GL_TEXTURE_2D, 0);
}

bool OffScreenRender::checkLoopThread() {
    return mLoopThreadCreate;
}

void OffScreenRender::envDestroy() {
    LOG_I("%s_envDestroy", TAG);
    if (mClientSurface != nullptr) { mClientSurface->release(mEglDisplay); }
    delete (mClientSurface);
    mClientSurface = nullptr;

    if (mClientWindow != nullptr) { ANativeWindow_release(mClientWindow); }
    mClientWindow = nullptr;

    if (mSurfaceTexture != nullptr) {
        ASurfaceTexture_release(mSurfaceTexture);
        mSurfaceTexture = nullptr;
    }

    if (mOffScreenWindowSurface != nullptr) { mOffScreenWindowSurface->release(mEglDisplay); }
    delete (mOffScreenWindowSurface);
    mOffScreenWindowSurface = nullptr;

    if (mOffScreenWindow != nullptr) { ANativeWindow_release(mOffScreenWindow); }
    mOffScreenWindow = nullptr;

    if (mOesTexture != 0) {
        glDeleteTextures(1, &mOesTexture);
        mOesTexture = 0;
    }

    if (mOesTexFilter != nullptr) { mOesTexFilter->destroy(); }
    delete (mOesTexFilter);
    mOesTexFilter = nullptr;

    if (mOesTexFilter != nullptr) { mOesTexFilter->destroy(); }
    delete (mOesTexFilter);
    mOesTexFilter = nullptr;

    if (mDefaultFilter != nullptr) { mDefaultFilter->destroy(); }
    delete (mDefaultFilter);
    mDefaultFilter = nullptr;

    if (mOffScreenFrameBuffer != 0) { glDeleteFramebuffers(1, &mOffScreenFrameBuffer); }
    if (mOffScreenTexture != 0) { glDeleteTextures(1, &mOffScreenTexture); }
    mOffScreenFrameBuffer = 0;
    mOffScreenTexture = 0;

    releaseEgl();
}

void OffScreenRender::envPause() {
    LOG_I("%s_envPause", TAG);
    if (mClientSurface != nullptr) { mClientSurface->release(mEglDisplay); }
    delete (mClientSurface);
    mClientSurface = nullptr;

    if (mClientWindow != nullptr) { ANativeWindow_release(mClientWindow); }
    mClientWindow = nullptr;

    if (mSurfaceTexture != nullptr) {
        ASurfaceTexture_release(mSurfaceTexture);
        mSurfaceTexture = nullptr;
    }

    if (mOffScreenWindowSurface != nullptr) { mOffScreenWindowSurface->release(mEglDisplay); }
    delete (mOffScreenWindowSurface);
    mOffScreenWindowSurface = nullptr;

    if (mOffScreenWindow != nullptr) { ANativeWindow_release(mOffScreenWindow); }
    mOffScreenWindow = nullptr;

    if (mOesTexture != 0) {
        glDeleteTextures(1, &mOesTexture);
        mOesTexture = 0;
    }
    if (mOesTexFilter != nullptr) { mOesTexFilter->onPause(); }
    if (mDefaultFilter != nullptr) { mDefaultFilter->onPause(); }

    if (mOffScreenFrameBuffer != 0) { glDeleteFramebuffers(1, &mOffScreenFrameBuffer); }
    if (mOffScreenTexture != 0) { glDeleteTextures(1, &mOffScreenTexture); }
    mOffScreenFrameBuffer = 0;
    mOffScreenTexture = 0;

    releaseEgl();
}

bool OffScreenRender::envResume(JNIEnv *env) {
    LOG_I("%s_envResume", TAG);
    mStatus = render::Status::STATUS_PREPARING;
    if (!prepareEgl()) { return false; }
    if (mOffScreenWindow == nullptr) {
        LOG_E("%s_envResume: off screen window is null", TAG);
        return false;
    }
    mOffScreenWindowSurface = new WindowSurface(mEglDisplay, mEglConfig, mOffScreenWindow);
    if (!mOffScreenWindowSurface->checkValid()) {
        LOG_E("%s_envResume: failed to create off screen window surface", TAG);
        return false;
    }
    mOffScreenWindowSurface->makeCurrent(mEglDisplay, mEglContext);
    prepareOffScreenFrameBuffer();
    createOesTexture();
    mStatus = render::Status::STATUS_PREPARED;
    notifyEnvPrepare(env, mJavaListener.get(), mOesTexture);
    return true;
}

void OffScreenRender::enqueueMessage(EventType what) {
    if (mEventQueue != nullptr) { mEventQueue->enqueue(EventMessage(what)); }
}

void OffScreenRender::handlePreDraw(JNIEnv *env) {
    if (!mOesTexFilter->initialized()) { mOesTexFilter->init(); }
    if (!mDefaultFilter->initialized()) { mDefaultFilter->init(); }
    updateTexImage();
}

void OffScreenRender::handleDraw(JNIEnv *env) {
    if (mClientSurface != nullptr) {
        mOffScreenWindowSurface->makeCurrent(mEglDisplay, mEglContext);
        glBindFramebuffer(GL_FRAMEBUFFER, mOffScreenFrameBuffer);
        //now data is filled in off screen texture
        mOesTexFilter->setTexMatrix(mTextureMatrix);
        mOesTexFilter->onDraw(mOesTexture);
        mOffScreenWindowSurface->swapBuffer(mEglDisplay);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        mClientSurface->makeCurrent(mEglDisplay, mEglContext);
        mDefaultFilter->onDraw(mOffScreenTexture);
        mClientSurface->swapBuffer(mEglDisplay);
    }
}

bool OffScreenRender::isPrepared() {
    return mEglContext != EGL_NO_CONTEXT;
}

void OffScreenRender::loop(JNIEnv *env) {
    LOG_I("%s_loop: enter", TAG);
    mLoopThreadCreate = true;
    mStatus = render::Status::STATUS_PREPARING;
    if (!prepareEgl()) { goto quit; }
    if (mOffScreenWindow == nullptr) {
        LOG_E("%s_loop: off screen window is null", TAG);
        goto quit;
    }
    mOffScreenWindowSurface = new WindowSurface(mEglDisplay, mEglConfig, mOffScreenWindow);
    if (!mOffScreenWindowSurface->checkValid()) {
        LOG_E("%s_loop: failed to create off screen window surface", TAG);
        goto quit;
    }
    mOffScreenWindowSurface->makeCurrent(mEglDisplay, mEglContext);
    prepareOffScreenFrameBuffer();
    createOesTexture();
    mStatus = render::Status::STATUS_PREPARED;
    notifyEnvPrepare(env, mJavaListener.get(), mOesTexture);
    for (;;) {
        EventMessage message = mEventQueue->dequeue();
        switch (message.what) {
            case EventType::EVENT_SET_CLIENT: {
                LOG_I("%s_loop: handle set client", TAG);
                if (mClientSurface != nullptr) {
                    mClientSurface->release(mEglDisplay);
                    delete mClientSurface;
                    mClientSurface = nullptr;
                }
                if (mClientWindow != nullptr) {
                    mClientSurface = new WindowSurface(mEglDisplay, mEglConfig, mClientWindow);
                    if (!mClientSurface->checkValid()) {
                        LOG_E("%s_loop: handle set client, failed to create client window surface", TAG);
                        goto quit;
                    }
                    glViewport(0, 0, mClientSurface->getWidth(), mClientSurface->getHeight());
                } else {
                    LOG_E("%s_loop: handle set client, client window is null", TAG);
                }
                break;
            }
            case EventType::EVENT_PAUSE: {
                LOG_I("%s_loop: handle pause", TAG);
                mStatus = render::Status::STATUS_PAUSE;
                envPause();
                break;
            }
            case EventType::EVENT_RESUME: {
                LOG_I("%s_loop: handle resume", TAG);
                mStatus = render::Status::STATUS_PREPARING;
                if (envResume(env)) {
                    break;
                } else {
                    goto quit;
                }
            }
            case EventType::EVENT_DRAW: {
                if (isPrepared()) {
                    mStatus = render::Status::STATUS_RUN;
                    handlePreDraw(env);
                    handleDraw(env);
                } else {
                    LOG_I("%s_loop: handle msg draw, env is not ready yet", TAG);
                }
                break;
            }
            case EventType::EVENT_QUIT: {
                LOG_I("%s_loop: handle msg quit", TAG);
                mStatus = render::Status::STATUS_DESTROY;
                envDestroy();
                goto quit;
            }
            default: {
                LOG_I("%s_loop: handle msg default, msg = %d", TAG,
                      static_cast<int8_t>(message.what));
                break;
            }
        }
    }
    quit:
    LOG_I("%s_loop: quit", TAG);
    notifyEnvRelease(env, mJavaListener.get());
    workBeforeKill(env);
    mLoopThreadCreate = false;
    delete (this);
}

void OffScreenRender::notifyEnvPrepare(JNIEnv *env, jobject listener, GLuint textureId) {
    if (listener != nullptr) {
        jclass listenerClass = env->GetObjectClass(listener);
        jmethodID methodId = env->GetMethodID(listenerClass, "onRenderEnvPrepare", "(I)V");
        env->CallVoidMethod(listener, methodId, textureId);
    }
}

void OffScreenRender::notifyEnvRelease(JNIEnv *env, jobject listener) {
    if (listener != nullptr) {
        jclass listenerClass = env->GetObjectClass(listener);
        jmethodID methodId = env->GetMethodID(listenerClass, "onRenderEnvRelease", "()V");
        env->CallVoidMethod(listener, methodId);
    }
}

bool OffScreenRender::prepareEgl() {
    LOG_I("%s_prepareEgl:", TAG);

    EGLint versionMinor;
    EGLint versionMajor;
    EGLint numConfigs;
    EGLint version;

    const EGLint contextAttribute[] = {EGL_CONTEXT_CLIENT_VERSION, 3, EGL_NONE};
    const EGLint configAttribute[] = {
            EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
            EGL_BLUE_SIZE, 8,
            EGL_GREEN_SIZE, 8,
            EGL_RED_SIZE, 8,
            EGL_ALPHA_SIZE, 8,
            EGL_DEPTH_SIZE, 8,
            EGL_STENCIL_SIZE, 8,
            EGL_RENDERABLE_TYPE, EGL_OPENGL_ES3_BIT_KHR,
            EGL_NONE
    };

    if (isPrepared()) {
        LOG_I("%s_prepareEgl: env is already prepared", TAG);
        return true;
    }

    mEglDisplay = eglGetDisplay(EGL_DEFAULT_DISPLAY);
    if (mEglDisplay == EGL_NO_DISPLAY) {
        LOG_E("%s_prepareEgl: failed to get display", TAG);
        goto fail;
    }
    if (!eglInitialize(mEglDisplay, &versionMajor, &versionMinor)) {
        LOG_E("%s_prepareEgl: failed to initialize display", TAG);
        goto fail;
    }
    LOG_I("%s_prepareEgl: initialize display, major = %d, minor = %d", TAG, versionMajor,
          versionMinor);

    if (!eglChooseConfig(mEglDisplay, configAttribute, &mEglConfig, 1, &numConfigs)) {
        LOG_E("%s_prepareEgl: failed to get config", TAG);
        goto fail;
    }
    mEglContext = eglCreateContext(mEglDisplay, mEglConfig, EGL_NO_CONTEXT, contextAttribute);
    if (mEglContext == EGL_NO_CONTEXT) {
        LOG_E("%s_prepareEgl: failed to egl context", TAG);
        goto fail;
    }
    eglQueryContext(mEglDisplay, mEglContext, GL_VERSION, &version);
    LOG_I("%s_prepareEgl: succeed to create context, version = %d", TAG, version);

    return true;

    fail:
    releaseEgl();
    return false;
}

void OffScreenRender::prepareOffScreenFrameBuffer() {
    LOG_I("%s_prepareOffScreenFrameBuffer", TAG);
    if (mOffScreenFrameBuffer != 0) { glDeleteFramebuffers(1, &mOffScreenFrameBuffer); }
    if (mOffScreenTexture != 0) { glDeleteTextures(1, &mOffScreenTexture); }

    glGenFramebuffers(1, &mOffScreenFrameBuffer);

    glGenTextures(1, &mOffScreenTexture);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glBindTexture(GL_TEXTURE_2D, mOffScreenTexture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, mOffScreenWindowSurface->getWidth(), mOffScreenWindowSurface->getHeight(), 0, GL_RGBA, GL_UNSIGNED_BYTE,nullptr);

    glBindFramebuffer(GL_FRAMEBUFFER, mOffScreenFrameBuffer);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, mOffScreenTexture,0);
    LOG_I("%s_prepareOffScreenFrameBuffer: check error = %d", TAG, glCheckFramebufferStatus(GL_FRAMEBUFFER));
    glBindTexture(GL_TEXTURE_2D, 0);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void OffScreenRender::releaseEgl() {
    LOG_I("%s_releaseEgl", TAG);
    if (mEglContext != EGL_NO_CONTEXT) {
        eglMakeCurrent(mEglDisplay, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
        eglDestroyContext(mEglDisplay, mEglContext);
        mEglContext = EGL_NO_CONTEXT;
    }
    if (mEglDisplay != EGL_NO_SURFACE) {
        eglTerminate(mEglDisplay);
        mEglDisplay = EGL_NO_DISPLAY;
    }
    mEglConfig = EGL_NO_CONFIG_KHR;
}

bool OffScreenRender::registerSelf(JNIEnv *env) {
    int count = sizeof(sJniMethods) / sizeof(sJniMethods[0]);
    jclass javaClass = env->FindClass(JAVA_CLASS_OFF_SCREEN_RENDER);
    if (!javaClass) {
        LOG_E("%s_registerSelf: failed to find class %s", TAG, JAVA_CLASS_OFF_SCREEN_RENDER);
        return false;
    }
    if (env->RegisterNatives(javaClass, sJniMethods, count) < 0) {
        LOG_E("%s_registerSelf: failed to register native methods %s", TAG,
              JAVA_CLASS_OFF_SCREEN_RENDER);
        return false;
    }
    if (render::gClassMap == nullptr) { render::createClassMap(); }
    jclass surfaceTextureClazz = env->FindClass(JAVA_CLASS_SURFACE_TEXTURE);
    if (surfaceTextureClazz != nullptr) {
        render::gClassMap->insert(std::pair<std::string, jobject>(JAVA_CLASS_SURFACE_TEXTURE,
                                                                  env->NewGlobalRef(
                                                                          surfaceTextureClazz)));
    }
    return true;
}

void OffScreenRender::setClientWindow(ANativeWindow *window) {
    mClientWindow = window;
}

void OffScreenRender::setOffScreenWindow(ANativeWindow *window) {
    mOffScreenWindow = window;
}

void OffScreenRender::setJavaListener(JNIEnv *env, jobject listener) {
    if (listener == nullptr) {
        LOG_E("%s_setJavaListener: input is nullptr", TAG);
        return;
    }
    if (mJavaListener != nullptr) {
        auto old = reinterpret_cast<jobject>(mJavaListener.release());
        env->DeleteGlobalRef(old);
    }
    mJavaListener.reset(env->NewGlobalRef(listener));
}

void OffScreenRender::setSurfaceTexture(JNIEnv *env, jobject texture) {
    if (texture == nullptr) {
        LOG_E("%s_setSurfaceTexture: input is nullptr", TAG);
        return;
    }
    if (mSurfaceTexture != nullptr) {
        ASurfaceTexture_release(mSurfaceTexture);
        mSurfaceTexture = nullptr;
    }
    mSurfaceTexture = ASurfaceTexture_fromSurfaceTexture(env, texture);
}

void OffScreenRender::workBeforeKill(JNIEnv *env) {
    mEventQueue->clear();
    delete mEventQueue;
    mEventQueue = nullptr;

    if (mJavaListener != nullptr) {
        auto old = reinterpret_cast<jobject>(mJavaListener.release());
        env->DeleteGlobalRef(old);
    }
}

void OffScreenRender::updateTexImage() {
    if (mSurfaceTexture == nullptr) {
        LOG_E("%s_updateTexImage: surface texture is null", TAG);
        return;
    }
    int res = ASurfaceTexture_updateTexImage(mSurfaceTexture);
    if (res == 0) {
        ASurfaceTexture_getTransformMatrix(mSurfaceTexture, mTextureMatrix);
    } else {
        LOG_E("%s_updateTexImage: error happens, result = %d", TAG, res);
    }
}
