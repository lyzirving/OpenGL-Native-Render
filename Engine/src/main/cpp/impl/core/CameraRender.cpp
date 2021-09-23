//
// Created by liuyuzhou on 2021/9/17.
//
#include "CameraRender.h"
#include "JniUtil.h"
#include "LogUtil.h"
#include "FilterFactory.h"
#include "MatrixUtil.h"

#define TAG "CameraRender"
#define JAVA_CLASS "com/render/engine/core/CameraRenderEngine"
#define JAVA_CLASS_RENDER_CAM_META_DATA "com/render/engine/camera/RenderCamMetadata"
#define JAVA_CLASS_SURFACE_TEXTURE "android/graphics/SurfaceTexture"

static std::map<jlong, jobject> gAdapters;
static std::map<std::string, jobject> gClassMap;

CameraRender::CameraRender() {
    mEvtQueue = new EventQueue;
    mWorkQueue = new WorkQueue;
    mEglCore = new RenderEglBase;
}

void *cameraRenderLoop(void *args) {
    auto *pRender = static_cast<CameraRender *>(args);
    JNIEnv *env = nullptr;
    if (!JniUtil::threadAttachJvm(EngineUtil::gJvm, &env)) {
        LogUtil::logI(TAG, {"renderLoop: failed to attach thread to jvm"});
        return nullptr;
    }
    pRender->render(env);
    JniUtil::detachThread(EngineUtil::gJvm);
    return nullptr;
}

static void nEnvBuildTexture(JNIEnv *env, jclass clazz, jlong ptr) {
    auto *pRender = reinterpret_cast<CameraRender *>(ptr);
    pRender->enqueueMessage(EventType::EVENT_BUILD_OES_TEXTURE);
}

static jlong nConstruct(JNIEnv *env, jclass clazz) {
    if (EngineUtil::gJvm == nullptr) { env->GetJavaVM(&EngineUtil::gJvm); }
    return reinterpret_cast<jlong>(new CameraRender);
}

static jboolean nEnvInitialized(JNIEnv *env, jclass clazz, jlong ptr) {
    auto *pRender = reinterpret_cast<CameraRender *>(ptr);
    return pRender->initialized();
}

static void nEnvPreviewChange(JNIEnv *env, jclass clazz, jlong ptr, jint previewWidth, jint previewHeight) {
    auto *pRender = reinterpret_cast<CameraRender *>(ptr);
    pRender->setPreview(previewWidth, previewHeight);
}

static void nEnvSurfaceCreate(JNIEnv *env, jclass clazz, jlong ptr, jobject surface, jobject adapter) {
    auto *pRender = reinterpret_cast<CameraRender *>(ptr);
    auto gUtil = GlUtil::self();
    if (!gUtil->mapContains(&gAdapters, ptr)) { gAdapters.insert(std::pair<jlong, jobject>(ptr, env->NewGlobalRef(adapter))); }
    if (!pRender->initialized()) {
        ANativeWindow *nativeWindow = ANativeWindow_fromSurface(env, surface);
        pRender->setNativeWindow(nativeWindow);
        pthread_t thread;
        pthread_create(&thread, nullptr, cameraRenderLoop, pRender);
        pthread_setname_np(thread, "camera-render-loop");
    }
}

static void nEnvSurfaceChange(JNIEnv *env, jclass clazz, jlong ptr, jint width, jint height) {
    auto *pRender = reinterpret_cast<CameraRender *>(ptr);
    pRender->setSize(width, height);
    pRender->enqueueMessage(EventType::EVENT_SURFACE_CHANGE);
}

static void nRequestRenderEnv(JNIEnv *env, jclass clazz, jlong ptr) {
    auto *pRender = reinterpret_cast<CameraRender *>(ptr);
    pRender->enqueueMessage(EventType::EVENT_DRAW);
}

static void nReleaseEnv(JNIEnv *env, jclass clazz, jlong ptr) {
    auto *pRender = reinterpret_cast<CameraRender *>(ptr);
    pRender->enqueueMessage(EventType::EVENT_QUIT);
}

static void nEnvOnPause(JNIEnv *env, jclass clazz, jlong ptr) {
    auto *pRender = reinterpret_cast<CameraRender *>(ptr);
    pRender->enqueueMessage(EventType::EVENT_PAUSE);
}

static void nEnvOnResume(JNIEnv *env, jclass clazz, jlong ptr, jobject surface) {
    auto *pRender = reinterpret_cast<CameraRender *>(ptr);
    ANativeWindow *nativeWindow = ANativeWindow_fromSurface(env, surface);
    pRender->setNativeWindow(nativeWindow);
    pRender->enqueueMessage(EventType::EVENT_RESUME);
}

static void nEnvSetSurfaceTexture(JNIEnv *env, jclass clazz, jlong ptr, jobject surfaceTexture) {
    auto *pRender = reinterpret_cast<CameraRender *>(ptr);
    pRender->setSurfaceTexture(env, surfaceTexture);
}

static void nEnvSetCamMetadata(JNIEnv *env, jclass clazz, jlong ptr, jobject data) {
    auto *pRender = reinterpret_cast<CameraRender *>(ptr);
    pRender->setCameraMetadata(env, data);
}

static JNINativeMethod sJniMethods[] = {
        {
                "nBuildTexture",           "(J)V",
                (void *) nEnvBuildTexture
        },
        {
                "nCreate",           "()J",
                (void *) nConstruct
        },
        {
                "nInitialized", "(J)Z",
                (void *) nEnvInitialized
        },
        {
                "nPreviewChange", "(JII)V",
                (void *) nEnvPreviewChange
        },
        {
                "nSurfaceCreate", "(JLandroid/view/Surface;Lcom/render/engine/core/RenderAdapter;)V",
                (void *) nEnvSurfaceCreate
        },
        {
                "nSurfaceChange", "(JII)V",
                (void *) nEnvSurfaceChange
        },
        {
                "nRequestRender",    "(J)V",
                (void *) nRequestRenderEnv
        },
        {
                "nRelease",          "(J)V",
                (void *) nReleaseEnv
        },
        {
                "nOnPause", "(J)V",
                (void *) nEnvOnPause
        },
        {
                "nOnResume", "(JLandroid/view/Surface;)V",
                (void *) nEnvOnResume
        },
        {
                "nSetSurfaceTexture", "(JLandroid/graphics/SurfaceTexture;)V",
                (void *) nEnvSetSurfaceTexture
        },
        {
                "nSetRenderCamMetadata", "(JLcom/render/engine/camera/RenderCamMetadata;)V",
                (void *) nEnvSetCamMetadata
        }
};

bool CameraRender::registerSelf(JNIEnv *env) {
    int count = sizeof(sJniMethods) / sizeof(sJniMethods[0]);
    jclass javaClass = env->FindClass(JAVA_CLASS);
    if (!javaClass) {
        LogUtil::logE(TAG, {"registerSelf: failed to find class ", JAVA_CLASS});
        return false;
    }
    if (env->RegisterNatives(javaClass, sJniMethods, count) < 0) {
        LogUtil::logE(TAG, {"registerSelf: failed to register native methods ", JAVA_CLASS});
        return false;
    }
    jclass surfaceTextureClazz = env->FindClass(JAVA_CLASS_SURFACE_TEXTURE);
    if (surfaceTextureClazz != nullptr) {
        gClassMap.insert(std::pair<std::string , jobject>(JAVA_CLASS_SURFACE_TEXTURE, env->NewGlobalRef(surfaceTextureClazz)));
    }
    jclass camDataClazz = env->FindClass(JAVA_CLASS_RENDER_CAM_META_DATA);
    if (camDataClazz != nullptr) {
        gClassMap.insert(std::pair<std::string , jobject>(JAVA_CLASS_RENDER_CAM_META_DATA, env->NewGlobalRef(camDataClazz)));
    }
    return true;
}

void CameraRender::buildOesTexture() {
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

void CameraRender::buildCameraTransMatrix() {
    if (mMetadata != nullptr && mCamMatrix == nullptr) {
        mCamMatrix = new GLfloat[16];
        MatrixUtil::setIdentityM(mCamMatrix, 0);
        if (mMetadata->frontType == CameraMetaData::LENS_FACING_FRONT) {
            MatrixUtil::rotate(mCamMatrix, 90, 0, 0, 1);
        } else if (mMetadata->frontType == CameraMetaData::LENS_FACING_BACK) {
            MatrixUtil::rotate(mCamMatrix, 270, 0, 0, 1);
        }
        float previewRatio = ((float)(mMetadata->previewWidth)) / ((float)(mMetadata->previewHeight));
        float viewRatio = ((float)(mWidth)) / ((float)(mHeight));
        LogUtil::logI(TAG, {"buildCameraTransMatrix: preview = ", std::to_string(previewRatio), ", view = ", std::to_string(viewRatio)});
        if (previewRatio > viewRatio) {
            MatrixUtil::scaleM(mCamMatrix, 0, viewRatio / previewRatio, 1, 1);
        }  else if (previewRatio < viewRatio) {
            MatrixUtil::scaleM(mCamMatrix, 0, 1, previewRatio / viewRatio,1);
        }
    }
}

void CameraRender::drawFrame() {
    if (mOesFilter != nullptr && mOesFilter->initialized() && mOesTexture != 0 && mMetadata != nullptr) {
        int drawCount = 0;
        mOesFilter->applyMatrix(mCamMatrix, 16);
        int lastTexture = mOesFilter->onDraw(mOesTexture);
        drawCount++;
        bool isOdd = (drawCount % 2) != 0;
        mScreenFilter->flip(false, isOdd);
        mScreenFilter->onDraw(lastTexture);
    }
    if (!mEglCore->swapBuffer()) {
        LogUtil::logI(TAG, {"drawFrame: failed to swap buffer"});
    }
}

void CameraRender::enqueueMessage(EventType what) {
    mEvtQueue->enqueueMessage(new EventMessage(what));
}

bool CameraRender::initialized() {
    return mStatus >= RenderStatus::STATUS_PREPARE && mStatus <= RenderStatus::STATUS_PAUSE;
}

void CameraRender::notifyEnvOesTextureCreate(JNIEnv *env, jobject listener, int oesTexture) {
    jclass listenerClass = env->GetObjectClass(listener);
    jmethodID methodId = env->GetMethodID(listenerClass, "onRenderOesTextureCreate", "(I)V");
    env->CallVoidMethod(listener, methodId, oesTexture);
}

void CameraRender::notifyEnvPrepare(JNIEnv *env, jobject listener) {
    jclass listenerClass = env->GetObjectClass(listener);
    jmethodID methodId = env->GetMethodID(listenerClass, "onRenderEnvPrepare", "()V");
    env->CallVoidMethod(listener, methodId);
}

void CameraRender::notifyEnvRelease(JNIEnv *env, jobject listener) {
    jclass listenerClass = env->GetObjectClass(listener);
    jmethodID methodId = env->GetMethodID(listenerClass, "onRenderEnvRelease", "()V");
    env->CallVoidMethod(listener, methodId);
}

void CameraRender::render(JNIEnv *env) {
    jobject listener = JniUtil::findListener(&gAdapters, reinterpret_cast<jlong>(this));
    if (!mEglCore->initEglEnv()) { goto quit; }
    mStatus = RenderStatus::STATUS_PREPARE;
    if (listener != nullptr) { notifyEnvPrepare(env, listener); }
    for (;;) {
        EventMessage &&message = mEvtQueue->dequeueMessage();
        switch (message.what) {
            case EventType::EVENT_SURFACE_CHANGE: {
                LogUtil::logI(TAG, {"render: handle message surface change"});
                mStatus = RenderStatus::STATUS_RUN;
                glViewport(0, 0, mWidth, mHeight);
                if (!mEglCore->swapBuffer()) {
                    LogUtil::logI(TAG, {"render: surface change, failed to swap buffer"});
                }
                break;
            }
            case EventType::EVENT_PAUSE: {
                LogUtil::logI(TAG, {"render: handle message pause"});
                mStatus = RenderStatus::STATUS_PAUSE;
                releaseBeforeEnvDestroy(env);
                if (mEglCore != nullptr) { mEglCore->release(); }
                mEvtQueue->clear();
                mWorkQueue->clear();
                break;
            }
            case EventType::EVENT_RESUME: {
                LogUtil::logI(TAG, {"render: handle message resume"});
                mStatus = RenderStatus::STATUS_RUN;
                if (mEglCore != nullptr) {
                    if (mEglCore->initEglEnv()) {
                        notifyEnvPrepare(env, listener);
                    } else {
                        goto quit;
                    }
                }
                break;
            }
            case EventType::EVENT_DRAW: {
                if (mEglCore != nullptr && mEglCore->valid()) {
                    mStatus = RenderStatus::STATUS_RUN;
                    updateTexImg(env);
                    runBeforeDraw();
                    drawFrame();
                } else {
                    LogUtil::logI(TAG, {"render: handle message draw, env is not valid"});
                }
                break;
            }
            case EventType::EVENT_BUILD_OES_TEXTURE: {
                LogUtil::logI(TAG, {"render: handle message build oes texture"});
                buildOesTexture();
                notifyEnvOesTextureCreate(env, listener, mOesTexture);
                break;
            }
            case EventType::EVENT_QUIT: {
                LogUtil::logI(TAG, {"render: handle message quit"});
                releaseBeforeEnvDestroy(env);
                mEvtQueue->clear();
                mWorkQueue->clear();
                mStatus = RenderStatus::STATUS_DESTROY;
                goto quit;
            }
            default: {
                LogUtil::logI(TAG, {"render: handle message default"});
                break;
            }
        }
    }
    quit:
    LogUtil::logI(TAG, {"render: quit render-loop"});
    if (listener != nullptr) { notifyEnvRelease(env, listener); }
    release(env);
    delete this;
}

void CameraRender::release(JNIEnv *env) {
    jobject adapter = JniUtil::removeListener(&gAdapters, reinterpret_cast<jlong>(this));
    if (adapter != nullptr) { env->DeleteGlobalRef(adapter); }
    if (mEglCore != nullptr) { mEglCore->release(); }
    delete mEvtQueue;
    delete mWorkQueue;
    mWorkQueue = nullptr;
    mEglCore = nullptr;
    mEvtQueue = nullptr;
}

void CameraRender::releaseBeforeEnvDestroy(JNIEnv* env) {
    LogUtil::logI(TAG, {"releaseBeforeEnvDestroy"});
    if (mOesTexture != 0) { glDeleteTextures(1, &mOesTexture); }
    mOesTexture = 0;

    if (mSurfaceTexture != nullptr) { env->DeleteGlobalRef(mSurfaceTexture); }
    mSurfaceTexture = nullptr;

    if (mScreenFilter != nullptr) {
        mScreenFilter->destroy();
        delete mScreenFilter;
    }
    mScreenFilter = nullptr;

    if (mOesFilter != nullptr) {
        mOesFilter->destroy();
        delete mOesFilter;
    }
    mOesFilter = nullptr;

    delete mMetadata;
    mMetadata = nullptr;

    delete[] mCamMatrix;
    mCamMatrix = nullptr;
}

void CameraRender::runBeforeDraw() {
    while (!mWorkQueue->empty()) {
        std::shared_ptr<WorkTask> task = std::make_shared<WorkTask>();
        if (mWorkQueue->dequeue(task)) { task->run(); }
    }
    if (mScreenFilter == nullptr) {
        LogUtil::logI(TAG, {"runBeforeDraw: screen filter init"});
        mScreenFilter = new ScreenFilter;
        mScreenFilter->setOutputSize(mWidth, mHeight);
        mScreenFilter->init();
    }
    if (mOesFilter == nullptr) {
        LogUtil::logI(TAG, {"runBeforeDraw: oes filter init"});
        mOesFilter = new OesFilter;
        mOesFilter->setPreviewSize(mPreviewWidth, mPreviewHeight);
        mOesFilter->setOutputSize(mWidth, mHeight);
        mOesFilter->init();
    }
    buildCameraTransMatrix();
    glClearColor(0.0, 0.0, 0.0, 1.0);
    glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
}

void CameraRender::setPreview(GLint previewWidth, GLint previewHeight) {
    mPreviewWidth = previewWidth;
    mPreviewHeight = previewHeight;
}

void CameraRender::setSize(GLint width, GLint height) {
    mWidth = width;
    mHeight = height;
}

void CameraRender::setNativeWindow(ANativeWindow *window) {
    mEglCore->setNativeWindow(window);
}

void CameraRender::setSurfaceTexture(JNIEnv* env, jobject surfaceTexture) {
    //the life cycle of mSurfaceTexture is the same as surfaceTexture java object
    //if the java object dies, mSurfaceTexture will be invalid, but that situation will not let the program terminate
    mSurfaceTexture = env->NewGlobalRef(surfaceTexture);
}

void CameraRender::setCameraMetadata(JNIEnv *env, jobject data) {
    delete mMetadata;
    auto javaClazz = JniUtil::find(&gClassMap, JAVA_CLASS_RENDER_CAM_META_DATA);
    if (javaClazz != nullptr) {
        jmethodID  method = env->GetMethodID(static_cast<jclass>(javaClazz), "getPreviewWidth", "()I");
        int previewWidth = env->CallIntMethod(data, method);

        method = env->GetMethodID(static_cast<jclass>(javaClazz), "getPreviewHeight", "()I");
        int previewHeight = env->CallIntMethod(data, method);

        method = env->GetMethodID(static_cast<jclass>(javaClazz), "getFrontType", "()I");
        int frontType = env->CallIntMethod(data, method);
        LogUtil::logI(TAG, {"setCameraMetadata: preview (", std::to_string(previewWidth), ",", std::to_string(previewHeight), "), front type = ", std::to_string(frontType)});
        mMetadata = new CameraMetaData;
        mMetadata->previewWidth = previewWidth;
        mMetadata->previewHeight = previewHeight;
        mMetadata->frontType = frontType;
    }
}

void CameraRender::updateTexImg(JNIEnv *env) {
    if (mSurfaceTexture != nullptr) {
        auto clazz = JniUtil::find(&gClassMap, JAVA_CLASS_SURFACE_TEXTURE);
        if (clazz != nullptr) {
            jmethodID methodId = env->GetMethodID(static_cast<jclass>(clazz), "updateTexImage", "()V");
            env->CallVoidMethod(mSurfaceTexture, methodId);
        } else {
            LogUtil::logI(TAG, {"updateTexImg: failed to find class ", JAVA_CLASS_SURFACE_TEXTURE});
        }
    }
}

