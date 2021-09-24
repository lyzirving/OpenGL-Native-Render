//
// Created by liuyuzhou on 2021/9/24.
//
#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>

#include "CamRender.h"
#include "LogUtil.h"
#include "JniUtil.h"
#include "MatrixUtil.h"

#define TAG "CamRender"

#define JAVA_CLASS_CAM_RENDER "com/render/engine/core/CamRenderEngine"
#define JAVA_CLASS_RENDER_CAM_META_DATA "com/render/engine/camera/RenderCamMetadata"
#define JAVA_CLASS_SURFACE_TEXTURE "android/graphics/SurfaceTexture"

void *envRenderLoop(void *args) {
    auto *pRender = static_cast<CamRender *>(args);
    JNIEnv *env = nullptr;
    if (!JniUtil::threadAttachJvm(render::gJvm, &env)) {
        LogUtil::logI(TAG, {"renderLoop: failed to attach thread to jvm"});
        return nullptr;
    }
    pRender->render(env);
    JniUtil::detachThread(render::gJvm);
    return nullptr;
}

static void nEnvBuildTexture(JNIEnv *env, jclass clazz, jlong ptr) {
    auto *pRender = reinterpret_cast<CamRender *>(ptr);
    pRender->enqueueMessage(EventType::EVENT_BUILD_OES_TEXTURE);
}

static jlong nEnvCreate(JNIEnv *env, jclass clazz) {
    if (render::gJvm == nullptr) { env->GetJavaVM(&render::gJvm); }
    return reinterpret_cast<jlong>(new CamRender);
}

static jboolean nEnvInitialized(JNIEnv *env, jclass clazz, jlong ptr) {
    auto *pRender = reinterpret_cast<CamRender *>(ptr);
    return pRender->initialized();
}

static void nEnvOnPause(JNIEnv *env, jclass clazz, jlong ptr) {
    auto *pRender = reinterpret_cast<CamRender *>(ptr);
    pRender->enqueueMessage(EventType::EVENT_PAUSE);
}

static void nEnvOnResume(JNIEnv *env, jclass clazz, jlong ptr, jobject surface) {
    auto *pRender = reinterpret_cast<CamRender *>(ptr);
    ANativeWindow *nativeWindow = ANativeWindow_fromSurface(env, surface);
    pRender->setNativeWindow(nativeWindow);
    pRender->enqueueMessage(EventType::EVENT_RESUME);
}

static void nEnvPreviewChange(JNIEnv *env, jclass clazz, jlong ptr, jint previewWidth, jint previewHeight) {
    auto *pRender = reinterpret_cast<CamRender*>(ptr);
    pRender->setPreview(previewWidth, previewHeight);
}

static void nEnvRequestRender(JNIEnv *env, jclass clazz, jlong ptr) {
    auto *pRender = reinterpret_cast<CamRender *>(ptr);
    pRender->enqueueMessage(EventType::EVENT_DRAW);
}

static void nEnvRelease(JNIEnv *env, jclass clazz, jlong ptr) {
    auto *pRender = reinterpret_cast<CamRender *>(ptr);
    pRender->enqueueMessage(EventType::EVENT_QUIT);
}

static void nEnvSurfaceCreate(JNIEnv *env, jclass clazz, jlong ptr, jobject surface, jobject adapter) {
    auto *pRender = reinterpret_cast<CamRender*>(ptr);
    pRender->setJavaListener(env, adapter);
    if (!pRender->initialized()) {
        ANativeWindow *nativeWindow = ANativeWindow_fromSurface(env, surface);
        pRender->setNativeWindow(nativeWindow);
        pthread_t thread;
        pthread_create(&thread, nullptr, envRenderLoop, pRender);
    }
}

static void nEnvSurfaceChange(JNIEnv *env, jclass clazz, jlong ptr, jint width, jint height) {
    auto *pRender = reinterpret_cast<CamRender *>(ptr);
    pRender->setSurfaceSize(width, height);
    pRender->enqueueMessage(EventType::EVENT_SURFACE_CHANGE);
}

static void nEnvSetCamMetadata(JNIEnv *env, jclass clazz, jlong ptr, jobject data) {
    auto *pRender = reinterpret_cast<CamRender *>(ptr);
    pRender->setCameraMetadata(env, data);
}

static void nEnvSetSurfaceTexture(JNIEnv *env, jclass clazz, jlong ptr, jobject surfaceTexture) {
    auto *pRender = reinterpret_cast<CamRender *>(ptr);
    pRender->setSurfaceTexture(env, surfaceTexture);
}

static JNINativeMethod sJniMethods[] = {
        {
                "nBuildTexture",           "(J)V",
                (void *) nEnvBuildTexture
        },
        {
                "nCreate",           "()J",
                (void *) nEnvCreate
        },
        {
                "nInitialized", "(J)Z",
                (void *) nEnvInitialized
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
                "nPreviewChange", "(JII)V",
                (void *) nEnvPreviewChange
        },
        {
                "nRequestRender",    "(J)V",
                (void *) nEnvRequestRender
        },
        {
                "nRelease",          "(J)V",
                (void *) nEnvRelease
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
                "nSetRenderCamMetadata", "(JLcom/render/engine/camera/RenderCamMetadata;)V",
                (void *) nEnvSetCamMetadata
        },
        {
                "nSetSurfaceTexture", "(JLandroid/graphics/SurfaceTexture;)V",
                (void *) nEnvSetSurfaceTexture
        }
};

bool CamRender::registerSelf(JNIEnv *env) {
    int count = sizeof(sJniMethods) / sizeof(sJniMethods[0]);
    jclass javaClass = env->FindClass(JAVA_CLASS_CAM_RENDER);
    if (!javaClass) {
        LogUtil::logE(TAG, {"registerSelf: failed to find class ", JAVA_CLASS_CAM_RENDER});
        return false;
    }
    if (env->RegisterNatives(javaClass, sJniMethods, count) < 0) {
        LogUtil::logE(TAG, {"registerSelf: failed to register native methods ", JAVA_CLASS_CAM_RENDER});
        return false;
    }
    if (render::gClassMap == nullptr) { render::createClassMap(); }
    jclass surfaceTextureClazz = env->FindClass(JAVA_CLASS_SURFACE_TEXTURE);
    if (surfaceTextureClazz != nullptr) {
        render::gClassMap->insert(std::pair<std::string , jobject>(JAVA_CLASS_SURFACE_TEXTURE, env->NewGlobalRef(surfaceTextureClazz)));
    }
    jclass camDataClazz = env->FindClass(JAVA_CLASS_RENDER_CAM_META_DATA);
    if (camDataClazz != nullptr) {
        render::gClassMap->insert(std::pair<std::string , jobject>(JAVA_CLASS_RENDER_CAM_META_DATA, env->NewGlobalRef(camDataClazz)));
    }
    return true;
}

bool CamRender::addBeautyFilter(const char *filterType, bool buildInitTask) {
    return false;
}

void CamRender::buildOesTexture() {
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

void CamRender::buildCameraTransMatrix() {
    if (mCamMetaData != nullptr && mCamMatrix == nullptr) {
        mCamMatrix = new GLfloat[16];
        MatrixUtil::setIdentityM(mCamMatrix, 0);
        if (mCamMetaData->frontType == EngineUtil::CameraMetaData::LENS_FACING_FRONT) {
            MatrixUtil::flip(mCamMatrix, true, false);
            MatrixUtil::rotate(mCamMatrix, 90, 0, 0, 1);
        } else if (mCamMetaData->frontType == EngineUtil::CameraMetaData::LENS_FACING_BACK) {
            MatrixUtil::rotate(mCamMatrix, 270, 0, 0, 1);
        }
        float previewRatio = ((float)(mCamMetaData->previewWidth)) / ((float)(mCamMetaData->previewHeight));
        float viewRatio = ((float)(mSurfaceWidth)) / ((float)(mSurfaceHeight));
        LogUtil::logI(TAG, {"buildCameraTransMatrix: preview = ", std::to_string(previewRatio), ", view = ", std::to_string(viewRatio)});
        if (previewRatio > viewRatio) {
            MatrixUtil::scaleM(mCamMatrix, 0, viewRatio / previewRatio, 1, 1);
        }  else if (previewRatio < viewRatio) {
            MatrixUtil::scaleM(mCamMatrix, 0, 1, previewRatio / viewRatio,1);
        }
    }
}

void CamRender::clearBeautyFilter() {}

void CamRender::drawFrame() {
    if (mOesFilter != nullptr && mOesFilter->initialized() && mOesTexture != 0 && mCamMetaData != nullptr) {
        int drawCount = 0;
        mOesFilter->applyMatrix(mCamMatrix, 16);
        int lastTexture = mOesFilter->onDraw(mOesTexture);
        drawCount++;
        bool isOdd = (drawCount % 2) != 0;
        mScreenFilter->flip(false, isOdd);
        mScreenFilter->onDraw(lastTexture);
    }
}

void CamRender::destroy(JNIEnv* env) {
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

    if (mOesTexture != 0) { glDeleteTextures(1, &mOesTexture); }
    mOesTexture = 0;

    if (mSurfaceTexture != nullptr) { env->DeleteGlobalRef(mSurfaceTexture); }
    mSurfaceTexture = nullptr;

    delete mCamMetaData;
    mCamMetaData = nullptr;

    delete[] mCamMatrix;
    mCamMatrix = nullptr;
}

void CamRender::handleOtherMessage(JNIEnv* env, EventType what) {
    switch (what) {
        case EventType::EVENT_BUILD_OES_TEXTURE: {
            LogUtil::logI(TAG, {"handleOtherMessage: build oes texture"});
            buildOesTexture();
            notifyEnvOesTextureCreate(env, mJavaListener, mOesTexture);
            break;
        }
        default: {
            LogUtil::logI(TAG, {"handleOtherMessage: default"});
            break;
        }
    }
}

void CamRender::handlePreDraw(JNIEnv *env) {
    updateTexImg(env);
    if (mScreenFilter == nullptr) {
        LogUtil::logI(TAG, {"handlePreDraw: screen filter init"});
        mScreenFilter = new ScreenFilter;
        mScreenFilter->setOutputSize(mSurfaceWidth, mSurfaceHeight);
        mScreenFilter->init();
    }
    if (mOesFilter == nullptr) {
        LogUtil::logI(TAG, {"handlePreDraw: oes filter init"});
        mOesFilter = new OesFilter;
        mOesFilter->setPreviewSize(mPreviewWidth, mPreviewHeight);
        mOesFilter->setOutputSize(mSurfaceWidth, mSurfaceHeight);
        mOesFilter->setCameraFaceFront(mCamMetaData->frontType);
        mOesFilter->init();
    }
    buildCameraTransMatrix();
    glClearColor(0.0, 0.0, 0.0, 1.0);
    glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
}

void CamRender::handlePostDraw(JNIEnv *env) {
    if (!mEglCore->swapBuffer()) {
        LogUtil::logI(TAG, {"drawFrame: failed to swap buffer"});
    }
}

void CamRender::handleRenderEnvPause(JNIEnv *env) {
    LogUtil::logI(TAG, {"handleRenderEnvPause"});
    destroy(env);
}

void CamRender::handleRenderEnvResume(JNIEnv *env) {
    //no implementation
}

void CamRender::handleRenderEnvDestroy(JNIEnv *env) {
    LogUtil::logI(TAG, {"handleRenderEnvDestroy"});
    destroy(env);
}

void CamRender::handleSurfaceChange(JNIEnv *env) {
    //no implementation
}

void CamRender::notifyEnvOesTextureCreate(JNIEnv *env, jobject listener, int oesTexture) {
    if (listener != nullptr) {
        jclass listenerClass = env->GetObjectClass(listener);
        jmethodID methodId = env->GetMethodID(listenerClass, "onRenderOesTextureCreate", "(I)V");
        env->CallVoidMethod(listener, methodId, oesTexture);
    }
}

void CamRender::release(JNIEnv *env) {
    BaseRender::release(env);
}

void CamRender::setCameraMetadata(JNIEnv *env, jobject data) {
    delete mCamMetaData;
    auto javaClazz = JniUtil::find(render::gClassMap, JAVA_CLASS_RENDER_CAM_META_DATA);
    if (javaClazz != nullptr) {
        jmethodID  method = env->GetMethodID(static_cast<jclass>(javaClazz), "getPreviewWidth", "()I");
        int previewWidth = env->CallIntMethod(data, method);

        method = env->GetMethodID(static_cast<jclass>(javaClazz), "getPreviewHeight", "()I");
        int previewHeight = env->CallIntMethod(data, method);

        method = env->GetMethodID(static_cast<jclass>(javaClazz), "getFrontType", "()I");
        int frontType = env->CallIntMethod(data, method);
        LogUtil::logI(TAG, {"setCameraMetadata: preview (", std::to_string(previewWidth), ",", std::to_string(previewHeight), "), front type = ", std::to_string(frontType)});
        mCamMetaData = new render::CameraMetaData;
        mCamMetaData->previewWidth = previewWidth;
        mCamMetaData->previewHeight = previewHeight;
        mCamMetaData->frontType = frontType;
    }
}

void CamRender::setPreview(GLint width, GLint height) {
    mPreviewWidth = width;
    mPreviewHeight = height;
}

void CamRender::setSurfaceTexture(JNIEnv *env, jobject surfaceTexture) {
    //the life cycle of mSurfaceTexture is the same as surfaceTexture java object
    //if the java object dies, mSurfaceTexture will be invalid, but that situation will not let the program terminate
    mSurfaceTexture = env->NewGlobalRef(surfaceTexture);
}

void CamRender::updateTexImg(JNIEnv *env) {
    if (mSurfaceTexture != nullptr) {
        auto clazz = JniUtil::find(render::gClassMap, JAVA_CLASS_SURFACE_TEXTURE);
        if (clazz != nullptr) {
            jmethodID methodId = env->GetMethodID(static_cast<jclass>(clazz), "updateTexImage", "()V");
            env->CallVoidMethod(mSurfaceTexture, methodId);
        } else {
            LogUtil::logI(TAG, {"updateTexImg: failed to find class ", JAVA_CLASS_SURFACE_TEXTURE});
        }
    }
}

