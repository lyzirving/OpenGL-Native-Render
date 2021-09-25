//
// Created by liuyuzhou on 2021/9/24.
//
#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>

#include "CamRender.h"
#include "FilterFactory.h"
#include "LogUtil.h"
#include "JniUtil.h"
#include "MatrixUtil.h"

#define TAG "CamRender"

#define JAVA_CLASS_CAM_RENDER "com/render/engine/camera/CamRenderEngine"
#define JAVA_CLASS_RENDER_CAM_META_DATA "com/render/engine/camera/RenderCamMetadata"
#define JAVA_CLASS_SURFACE_TEXTURE "android/graphics/SurfaceTexture"

static void nEnvBuildTexture(JNIEnv *env, jclass clazz, jlong ptr) {
    auto *pRender = reinterpret_cast<CamRender *>(ptr);
    pRender->enqueueMessage(EventType::EVENT_BUILD_OES_TEXTURE);
}

static jlong nEnvCreate(JNIEnv *env, jclass clazz) {
    return reinterpret_cast<jlong>(new CamRender);
}

static void nEnvPreviewChange(JNIEnv *env, jclass clazz, jlong ptr, jint previewWidth, jint previewHeight) {
    auto *pRender = reinterpret_cast<CamRender*>(ptr);
    pRender->setPreview(previewWidth, previewHeight);
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
                "nPreviewChange", "(JII)V",
                (void *) nEnvPreviewChange
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
        if (mCamMetaData->frontType == render::CameraMetaData::LENS_FACING_FRONT) {
            MatrixUtil::flip(mCamMatrix, true, false);
            MatrixUtil::rotate(mCamMatrix, 90, 0, 0, 1);
        } else if (mCamMetaData->frontType == render::CameraMetaData::LENS_FACING_BACK) {
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

void CamRender::drawFrame() {
    if (mOesFilter != nullptr && mOesFilter->initialized() && mOesTexture != 0 && mCamMetaData != nullptr) {
        int drawCount = 0;
        mOesFilter->applyMatrix(mCamMatrix, 16);
        int lastTexture = mOesFilter->onDraw(mOesTexture);
        drawCount++;
        if (mBeautyFilterGroup != nullptr) {
            lastTexture = mBeautyFilterGroup->onDraw(lastTexture);
            drawCount += mBeautyFilterGroup->filterSize();
        }
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

    if (mBeautyFilterGroup != nullptr) {
        mBeautyFilterGroup->destroy();
        delete mBeautyFilterGroup;
    }
    mBeautyFilterGroup = nullptr;

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
    pause(env);
}

void CamRender::handleRenderEnvResume(JNIEnv *env) {
    //resume the beauty filters if need
    if (mBeautyFilterGroup != nullptr) {
        mBeautyFilterGroup->setOutputSize(mSurfaceWidth, mSurfaceHeight);
        mBeautyFilterGroup->init();
    }
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

void CamRender::pause(JNIEnv* env) {
    if (mBeautyFilterGroup != nullptr) {
        //should not destroy filter group when env is paused
        //because we should memorise the inner data, and resume
        mBeautyFilterGroup->onPause();
    }

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

