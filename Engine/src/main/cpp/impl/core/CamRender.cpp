//
// Created by liuyuzhou on 2021/9/24.
//
#include <GLES3/gl3.h>
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

#define NUM_FACE_CTRL_PT 4

static void nEnvBuildTexture(JNIEnv *env, jclass clazz, jlong ptr) {
    auto *pRender = reinterpret_cast<CamRender *>(ptr);
    pRender->enqueueMessage(EventType::EVENT_BUILD_OES_TEXTURE);
}

static jlong nEnvCreate(JNIEnv *env, jclass clazz) {
    return reinterpret_cast<jlong>(new CamRender);
}

static void nEnvDetect(JNIEnv *env, jclass clazz, jlong ptr, jboolean start) {
    auto *pRender = reinterpret_cast<CamRender *>(ptr);
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

static void* envHandleLandMarkDetect(void* env, void* arg0, int argNum, ...) {
    auto *pRender = reinterpret_cast<CamRender *>(arg0);
    if (argNum == NUM_FACE_CTRL_PT) {
        Point* pts[NUM_FACE_CTRL_PT];
        Point* pt = nullptr;
        std::va_list args;
        va_start(args, argNum);
        while (argNum--) {
            pt = va_arg(args, Point*);
            pts[NUM_FACE_CTRL_PT - 1 - argNum] = pt;
        }
        va_end(args);
        pRender->handleLandMarkTrack(pts[0], pts[1], pts[2], pts[3]);
    } else {
        LogUtil::logI(TAG, {"envHandleLandMarkDetect: invalid args num = ", std::to_string(argNum)});
    }
    return nullptr;
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

void CamRender::adjustProperty(const char *filterType, const char *property, int progress) {
    BaseRender::adjustProperty(filterType, property, progress);
    if (filterType == nullptr || std::strlen(filterType) == 0) {
        LogUtil::logI(TAG, {"adjustProperty: filter type is invalid"});
        return;
    }
    if (property == nullptr || std::strlen(property) == 0) {
        LogUtil::logI(TAG, {"adjustProperty: filter prop is invalid"});
        return;
    }
    if (std::strcmp(render::FILTER_FACE_LIFT, filterType) == 0) {
        if (std::strcmp(render::FILTER_PROP_FACE_LIFT, property) == 0 && mFaceLiftFilter != nullptr && mFaceLiftFilter->initialized()) {
            mFaceLiftFilter->adjust(progress);
        }
    } else {
        LogUtil::logI(TAG, {"adjustProperty: unknown filter ", filterType});
    }
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

void CamRender::drawFrame() {
    if (mOesFilter != nullptr && mOesFilter->initialized() && mOesTexture != 0 && mCamMetaData != nullptr) {
        int drawCount = 0;
        int lastTexture = mOesFilter->onDraw(mOesTexture);
        drawCount++;
        notifyShareEnvDraw(getContentTexture(), drawCount);
        handleDownloadPixel(reinterpret_cast<GLuint *>(&lastTexture), drawCount);
        if (mBeautifyFaceFilter != nullptr && mBeautifyFaceFilter->initialized()) {
            lastTexture = mBeautifyFaceFilter->onDraw(lastTexture);
        }
        if (mBeautyFilterGroup != nullptr && mBeautyFilterGroup->initialized()) {
            lastTexture = mBeautyFilterGroup->onDraw(lastTexture);
            drawCount += mBeautyFilterGroup->filterSize();
        }
        bool isOdd = (drawCount % 2) != 0;
        mScreenFilter->flip(false, isOdd);
        mScreenFilter->onDraw(lastTexture);
    }
}

void CamRender::downloadPreview(GLuint frameBuffer) {
    if (frameBuffer == GL_NONE) {
        LogUtil::logI(TAG, {"downloadPreview: frame buffer is invalid"});
        return;
    }
    glBindFramebuffer(GL_FRAMEBUFFER, frameBuffer);

    glBindBuffer(GL_PIXEL_PACK_BUFFER, mDownloadBuffer[mDownloadFreeIndex]);
    glReadPixels(0, 0, mSurfaceWidth, mSurfaceHeight, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);

    unsigned char* pixelData = nullptr;
    int nextIndex = mDownloadFreeIndex == 0 ? 1 : 0;
    glBindBuffer(GL_PIXEL_PACK_BUFFER, mDownloadBuffer[nextIndex]);
    pixelData = static_cast<unsigned char *>(glMapBufferRange(GL_PIXEL_PACK_BUFFER, 0,
                                                              mSurfaceWidth * mSurfaceHeight * 4,
                                                              GL_MAP_READ_BIT));
    //use data before unmap buffer;
    if (pixelData) {
        switch (mDownloadMode) {
            case render::DownloadMode::MODE_FACE_DETECT: {
                mCamFaceDetector->enqueueImg(pixelData, mSurfaceWidth, mSurfaceHeight, 4, EventType::EVENT_FACE_LAND_MARK_TRACK);
                break;
            }
            case render::DownloadMode::MODE_WRITE_PNG: {
                mCamFaceDetector->enqueueImg(pixelData, mSurfaceWidth, mSurfaceHeight, 4, EventType::EVENT_WRITE_PNG);
                break;
            }
            case render::DownloadMode::MODE_NONE:
            default: {
                break;
            }
        }
    }
    glUnmapBuffer(GL_PIXEL_PACK_BUFFER);
    glBindBuffer(GL_PIXEL_PACK_BUFFER, GL_NONE);
    glBindFramebuffer(GL_FRAMEBUFFER, GL_NONE);

    mDownloadFreeIndex = mDownloadFreeIndex == 0 ? 1 : 0;
}

GLuint CamRender::getContentTexture() {
    if (mOesFilter != nullptr && mOesFilter->initialized()) {
        return mOesFilter->getContentTexture();
    } else {
        LogUtil::logI(TAG, {"getContentTexture: oes filter is not valid"});
        return 0;
    }
}

void CamRender::handleEnvPrepare(JNIEnv *env) {
    if (mCamFaceDetector == nullptr) {
        mCamFaceDetector = new CamFaceDetector();
        mCamFaceDetector->setLandMarkDetectCallback(envHandleLandMarkDetect);
        mCamFaceDetector->setFaceListener(this);
    }
    mCamFaceDetector->prepare(env);
}

void CamRender::handleDownloadPixel(GLuint* inputTexture, int drawCount) {
    if (mCamFaceDetector != nullptr && mCamFaceDetector->isRunning() && mDownloadFilter != nullptr && mDownloadFilter->initialized()) {
        mDownloadFilter->onDraw(*inputTexture);
        downloadPreview(mDownloadFilter->getFrameBuffer());
        if (mFaceLiftFilter != nullptr && mFaceLiftFilter->pointValid()) {
            if (drawCount % 2 != 0) { *inputTexture = mPlaceHolderFilter->onDraw(*inputTexture); }
            *inputTexture = mFaceLiftFilter->onDraw(*inputTexture);
        }
    }
}

void CamRender::handleFaceTrackStart(JNIEnv *env) {}

void CamRender::handleFaceTrackStop(JNIEnv *env) {}

void CamRender::handleLandMarkTrack(Point *lhsDst, Point *lhsCtrl, Point *rhsDst, Point *rhsCtrl) {
    if (mFaceLiftFilter != nullptr) {
        mFaceLiftFilter->setPts(lhsDst, lhsCtrl, rhsDst, rhsCtrl);
    } else {
        LogUtil::logI(TAG, {"handleLandMarkTrack: filter is nullptr"});
    }
}

void CamRender::handleOtherMessage(JNIEnv* env, const EventMessage& msg) {
    switch (msg.what) {
        case EventType::EVENT_BUILD_OES_TEXTURE: {
            LogUtil::logI(TAG, {"handleOtherMessage: build oes texture"});
            buildOesTexture();
            notifyEnvOesTextureCreate(env, GET_LISTENER, mOesTexture);
            break;
        }
        case EventType::EVENT_FACE_TRACK_START: {
            LogUtil::logI(TAG, {"handleOtherMessage: face track start"});
            handleFaceTrackStart(env);
            break;
        }
        case EventType::EVENT_FACE_TRACK_STOP: {
            LogUtil::logI(TAG, {"handleOtherMessage: face track stop"});
            handleFaceTrackStop(env);
            break;
        }
        default: {
            LogUtil::logI(TAG, {"handleOtherMessage: default"});
            break;
        }
    }
}

void CamRender::handlePreDraw(JNIEnv *env) {
    if (mPlaceHolderFilter == nullptr) {
        LogUtil::logI(TAG, {"handlePreDraw: place holder filter init"});
        mPlaceHolderFilter = new PlaceHolderFilter;
        mPlaceHolderFilter->setOutputSize(mSurfaceWidth, mSurfaceHeight);
        mPlaceHolderFilter->init();
    }
    if (mOesFilter == nullptr) {
        LogUtil::logI(TAG, {"handlePreDraw: oes filter init"});
        mOesFilter = new OesFilter;
        mOesFilter->setPreviewSize(mPreviewWidth, mPreviewHeight);
        mOesFilter->setOutputSize(mSurfaceWidth, mSurfaceHeight);
        mOesFilter->setCameraFaceFront(mCamMetaData->frontType);
        mOesFilter->calculateMatrix();
        mOesFilter->init();
    }
    if (mDownloadFilter != nullptr && !mDownloadFilter->initialized()) {
        mDownloadFilter->setOutputSize(mSurfaceWidth, mSurfaceHeight);
        mDownloadFilter->init();
    }
    if (mFaceLiftFilter != nullptr && !mFaceLiftFilter->initialized()) {
        mFaceLiftFilter->setOutputSize(mSurfaceWidth, mSurfaceHeight);
        mFaceLiftFilter->init();
    }
    updateTexImg(env);
    glClearColor(0.0, 0.0, 0.0, 1.0);
    glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
}

void CamRender::handlePostDraw(JNIEnv *env) {
    if (!mEglCore->swapBuffer()) {
        LogUtil::logI(TAG, {"handlePostDraw: failed to swap buffer"});
    }
}

void CamRender::handleRenderEnvPause(JNIEnv *env) {
    LogUtil::logI(TAG, {"handleRenderEnvPause"});

    if (mPlaceHolderFilter != nullptr) {
        mPlaceHolderFilter->destroy();
        delete mPlaceHolderFilter;
    }
    mPlaceHolderFilter = nullptr;

    if (mOesFilter != nullptr) {
        mOesFilter->destroy();
        delete mOesFilter;
    }
    mOesFilter = nullptr;

    if (mDownloadFilter != nullptr) { mDownloadFilter->onPause(); }

    if (mFaceLiftFilter != nullptr) { mFaceLiftFilter->onPause(); }

    if (mOesTexture != 0) { glDeleteTextures(1, &mOesTexture); }
    mOesTexture = 0;

    if (mSurfaceTexture != nullptr) { env->DeleteGlobalRef(mSurfaceTexture); }
    mSurfaceTexture = nullptr;

    if (mDownloadBuffer != nullptr) {
        glDeleteBuffers(2, mDownloadBuffer);
        delete[] mDownloadBuffer;
        mDownloadBuffer = nullptr;
    }
    mDownloadFreeIndex = 0;

    delete mCamMetaData;
    mCamMetaData = nullptr;
}

void CamRender::handleRenderEnvResume(JNIEnv *env) {}

void CamRender::handleRenderEnvDestroy(JNIEnv *env) {
    LogUtil::logI(TAG, {"handleRenderEnvDestroy"});

    if (mPlaceHolderFilter != nullptr) {
        mPlaceHolderFilter->destroy();
        delete mPlaceHolderFilter;
    }
    mPlaceHolderFilter = nullptr;

    if (mOesFilter != nullptr) {
        mOesFilter->destroy();
        delete mOesFilter;
    }
    mOesFilter = nullptr;

    if (mDownloadFilter != nullptr) {
        mDownloadFilter->destroy();
        delete mDownloadFilter;
    }
    mDownloadFilter = nullptr;

    if (mFaceLiftFilter != nullptr) {
        mFaceLiftFilter->destroy();
        delete mFaceLiftFilter;
    }
    mFaceLiftFilter = nullptr;

    if (mOesTexture != 0) { glDeleteTextures(1, &mOesTexture); }
    mOesTexture = 0;

    if (mSurfaceTexture != nullptr) { env->DeleteGlobalRef(mSurfaceTexture); }
    mSurfaceTexture = nullptr;

    if (mCamFaceDetector != nullptr) {
        mCamFaceDetector->quitAndWait();
    }
    delete mCamFaceDetector;
    mCamFaceDetector = nullptr;

    delete mCamMetaData;
    mCamMetaData = nullptr;
}

void CamRender::handleSurfaceChange(JNIEnv *env) {
    if (mDownloadBuffer != nullptr) {
        glDeleteBuffers(2, mDownloadBuffer);
        delete[] mDownloadBuffer;
    }
    mDownloadBuffer = new GLuint[2];
    glGenBuffers(2, mDownloadBuffer);

    int bufSize = mSurfaceWidth * mSurfaceHeight * 4;
    glBindBuffer(GL_PIXEL_PACK_BUFFER, mDownloadBuffer[0]);
    glBufferData(GL_PIXEL_PACK_BUFFER, bufSize, nullptr, GL_STREAM_READ);

    glBindBuffer(GL_PIXEL_PACK_BUFFER, mDownloadBuffer[1]);
    glBufferData(GL_PIXEL_PACK_BUFFER, bufSize, nullptr, GL_STREAM_READ);

    glBindBuffer(GL_PIXEL_PACK_BUFFER, GL_NONE);

    mDownloadFreeIndex = 0;
}

void CamRender::notifyEnvOesTextureCreate(JNIEnv *env, jobject listener, int oesTexture) {
    if (listener != nullptr) {
        jclass listenerClass = env->GetObjectClass(listener);
        jmethodID methodId = env->GetMethodID(listenerClass, "onRenderOesTextureCreate", "(I)V");
        env->CallVoidMethod(listener, methodId, oesTexture);
    }
}

void CamRender::setCameraMetadata(JNIEnv *env, jobject data) {
    delete mCamMetaData;
    auto javaClazz = JniUtil::self()->find(render::gClassMap, JAVA_CLASS_RENDER_CAM_META_DATA);
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

void CamRender::trackFace(bool start) {
    if (start) {
        if (mDownloadFilter == nullptr) {
            mDownloadFilter = new DownloadPixelFilter;
            mDownloadFilter->setOutputSize(mSurfaceWidth, mSurfaceHeight);
            std::shared_ptr<WorkTask> task = std::make_shared<FilterInitTask>();
            task->setObj(mDownloadFilter);
            mWorkQueue->enqueue(task);
        }
        if (mFaceLiftFilter == nullptr) {
            mFaceLiftFilter = new FaceLiftFilter;
            mFaceLiftFilter->setOutputSize(mSurfaceWidth, mSurfaceHeight);
            std::shared_ptr<WorkTask> task = std::make_shared<FilterInitTask>();
            task->setObj(mFaceLiftFilter);
            mWorkQueue->enqueue(task);
        }
    }
    mCamFaceDetector->execute(start);
}

void CamRender::updateTexImg(JNIEnv *env) {
    if (mSurfaceTexture != nullptr) {
        auto clazz = JniUtil::self()->find(render::gClassMap, JAVA_CLASS_SURFACE_TEXTURE);
        if (clazz != nullptr) {
            jmethodID methodId = env->GetMethodID(static_cast<jclass>(clazz), "updateTexImage", "()V");
            env->CallVoidMethod(mSurfaceTexture, methodId);
        } else {
            LogUtil::logI(TAG, {"updateTexImg: failed to find class ", JAVA_CLASS_SURFACE_TEXTURE});
        }
    }
}




