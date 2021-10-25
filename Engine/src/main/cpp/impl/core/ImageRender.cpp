//
// Created by lyzirving on 2021/9/25.
//
#include "ImageRender.h"
#include "LogUtil.h"
#include "ModelLoader.h"

#define TAG "ImageRender"
#define JAVA_CLASS_IMAGE_RENDER "com/render/engine/img/ImageRenderEngine"
static const int RENDER_NO_TEXTURE = 0;

#define NUM_FACE_CTRL_PT 4

class EnvNotPreparedException : public std::exception {
public:
    const char *what() const noexcept override { return "Env is not prepared"; }
};

static jlong nEnvCreate(JNIEnv *env, jclass clazz) {
    return reinterpret_cast<jlong>(new ImageRender);
}

static void nEnvSetResource(JNIEnv *env, jclass clazz, jlong ptr, jobject bitmap) {
    auto *pRender = reinterpret_cast<ImageRender *>(ptr);
    pRender->setResource(env, bitmap);
}

static void* envHandleLandMarkDetect(void* env, void* arg0, int argNum, ...) {
    auto *pRender = reinterpret_cast<ImageRender *>(arg0);
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
        pRender->handleFaceLiftLandMarkTrack(pts[0], pts[1], pts[2], pts[3]);
        pRender->enqueueMessage(EventType::EVENT_DRAW);
        pRender->notifyImageLandMarkTrackFinish(static_cast<JNIEnv *>(env));
    } else {
        LogUtil::logI(TAG, {"envHandleLandMarkDetect: invalid feature pt num = ", std::to_string(argNum)});
    }
    return nullptr;
}

static void* envHandleTrackPause(void* env, void* arg0) {
    LogUtil::logI(TAG, {"envHandleTrackPause"});
    auto *pRender = reinterpret_cast<ImageRender *>(arg0);
    pRender->enqueueMessage(EventType::EVENT_DRAW);
    return nullptr;
}

static JNINativeMethod sJniMethods[] = {
        {
                "nCreate",           "()J",
                (void *) nEnvCreate
        },
        {
                "nSetResource",    "(JLandroid/graphics/Bitmap;)V",
                (void *) nEnvSetResource
        }
};

bool ImageRender::registerSelf(JNIEnv *env) {
    int count = sizeof(sJniMethods) / sizeof(sJniMethods[0]);
    jclass javaClass = env->FindClass(JAVA_CLASS_IMAGE_RENDER);
    if (!javaClass) {
        LogUtil::logE(TAG, {"registerSelf: failed to find class ", JAVA_CLASS_IMAGE_RENDER});
        return false;
    }
    if (env->RegisterNatives(javaClass, sJniMethods, count) < 0) {
        LogUtil::logE(TAG, {"registerSelf: failed to register native methods ", JAVA_CLASS_IMAGE_RENDER});
        return false;
    }
    return true;
}

void ImageRender::adjustProperty(const char *filterType, const char *property, int progress) {
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

void ImageRender::drawFrame() {
    if (mBackgroundFilter != nullptr) {
        LogUtil::logI(TAG, {"drawFrame"});
        //background filter has content texture on its own, so you don't need to pass texture to it;
        int drawCount = 0;
        int lastTexture = mBackgroundFilter->onDraw(RENDER_NO_TEXTURE);
        drawCount++;
        drawFaceLift(reinterpret_cast<GLuint *>(&lastTexture), drawCount);
        if (mBeautifyFaceFilter != nullptr && mBeautifyFaceFilter->initialized()) {
            lastTexture = mBeautifyFaceFilter->onDraw(lastTexture);
        }
        if (mBeautyFilterGroup != nullptr && mBeautyFilterGroup->initialized()) {
            lastTexture = mBeautyFilterGroup->onDraw(lastTexture);
            drawCount += mBeautyFilterGroup->filterSize();
        }
        lastTexture = mMaskFilter->onDraw(lastTexture);
        drawCount++;
        bool isOdd = (drawCount % 2) != 0;
        LogUtil::logI(TAG, {"drawFrame: draw screen, draw count = ", std::to_string(drawCount)});
        mScreenFilter->flip(false, isOdd);
        mScreenFilter->onDraw(lastTexture);
    } else {
        LogUtil::logI(TAG, {"drawFrame: background filter is nullptr"});
    }
}

void ImageRender::drawFaceLift(GLuint *inputTexture, int drawCount) {
    if (mFaceLiftFilter != nullptr && mFaceLiftFilter->pointValid()
        && mImageFaceDetector != nullptr && mImageFaceDetector->isRunning()) {
        if (drawCount % 2 != 0) { *inputTexture = mPlaceHolderFilter->onDraw(*inputTexture); }
        *inputTexture = mFaceLiftFilter->onDraw(*inputTexture);
    }
}

void ImageRender::handleEnvPrepare(JNIEnv *env) {
    if (mImageFaceDetector == nullptr) {
        mImageFaceDetector = new ImageFaceDetector;
        mImageFaceDetector->setLandMarkDetectCallback(envHandleLandMarkDetect);
        mImageFaceDetector->setTrackPauseCallback(envHandleTrackPause);
        mImageFaceDetector->setFaceListener(this);
    }
    mImageFaceDetector->prepare(env);

    ModelLoader loader;
    mModelFilter = new ModelFilter;
    mModelFilter->setObjGroup(loader.buildModel("tiger_nose"));

    std::shared_ptr<WorkTask> task = std::make_shared<FilterInitTask>();
    task->setObj(mModelFilter);
    mWorkQueue->enqueue(task);
}

void ImageRender::handleFaceLiftLandMarkTrack(Point *lhsDst, Point *lhsCtrl, Point *rhsDst, Point *rhsCtrl) {
    if (mPlaceHolderFilter == nullptr) { mPlaceHolderFilter = new PlaceHolderFilter; }
    mPlaceHolderFilter->setOutputSize(mSurfaceWidth, mSurfaceHeight);
    if (!mPlaceHolderFilter->initialized()) {
        std::shared_ptr<WorkTask> task = std::make_shared<FilterInitTask>();
        task->setObj(mPlaceHolderFilter);
        mWorkQueue->enqueue(task);
    }
    if (mFaceLiftFilter == nullptr) { mFaceLiftFilter = new FaceLiftFilter; }
    mFaceLiftFilter->setOutputSize(mSurfaceWidth, mSurfaceHeight);
    if (!mFaceLiftFilter->initialized()) {
        std::shared_ptr<WorkTask> task = std::make_shared<FilterInitTask>();
        task->setObj(mFaceLiftFilter);
        mWorkQueue->enqueue(task);
    }
    mFaceLiftFilter->setLhsDstPt(lhsDst->x, lhsDst->y);
    mFaceLiftFilter->setLhsCtrlPt(lhsCtrl->x, lhsCtrl->y);
    mFaceLiftFilter->setRhsDstPt(rhsDst->x, rhsDst->y);
    mFaceLiftFilter->setRhsCtrlPt(rhsCtrl->x, rhsCtrl->y);
}

void ImageRender::handleOtherMessage(JNIEnv *env, const EventMessage& msg) {
    switch (msg.what) {
        case EventType::EVENT_FACE_TRACK_START: {
            LogUtil::logI(TAG, {"handleOtherMessage: face track start"});
            int delay = msg.arg0;
            if (delay > 0) { std::this_thread::sleep_for(std::chrono::milliseconds(delay)); }
            runTaskPreDraw();
            handleDownloadRawPreview();
            break;
        }
        default: {
            LogUtil::logI(TAG, {"handleOtherMessage: default"});
            break;
        }
    }
}

void ImageRender::handlePreDraw(JNIEnv *env) {
    glClearColor(1.0, 1.0, 1.0, 1.0);
    glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
}

void ImageRender::handlePostDraw(JNIEnv *env) {
    if (!mEglCore->swapBuffer()) {
        LogUtil::logI(TAG, {"handlePostDraw: failed to swap buffer"});
    }
}

void ImageRender::handleRenderEnvPause(JNIEnv *env) {
    if (mBackgroundFilter != nullptr) { mBackgroundFilter->onPause(); }
    if (mMaskFilter != nullptr) { mMaskFilter->onPause(); }
    if (mFaceLiftFilter != nullptr) { mFaceLiftFilter->onPause(); }
    if (mPlaceHolderFilter != nullptr) { mPlaceHolderFilter->onPause(); }
    if (mImageFaceDetector != nullptr) { mImageFaceDetector->execute(false); }
    if (mDownloadBuffer != 0) {
        glDeleteBuffers(1, &mDownloadBuffer);
        mDownloadBuffer = 0;
    }
}

void ImageRender::handleRenderEnvResume(JNIEnv *env) {
    if (mImageFaceDetector != nullptr) { mImageFaceDetector->execute(true); }
    if (mPlaceHolderFilter != nullptr && !mPlaceHolderFilter->initialized()) {
        mPlaceHolderFilter->setOutputSize(mSurfaceWidth, mSurfaceHeight);
        mPlaceHolderFilter->init();
    }
    if (mFaceLiftFilter != nullptr && !mFaceLiftFilter->initialized()) {
        mFaceLiftFilter->setOutputSize(mSurfaceWidth, mSurfaceHeight);
        mFaceLiftFilter->init();
    }
}

void ImageRender::handleRenderEnvDestroy(JNIEnv *env) {
    if (mMaskFilter != nullptr) { mMaskFilter->destroy(); }
    if (mBackgroundFilter != nullptr) { mBackgroundFilter->destroy(); }
    if (mFaceLiftFilter != nullptr) { mFaceLiftFilter->destroy(); }
    if (mPlaceHolderFilter != nullptr) { mPlaceHolderFilter->destroy(); }
    if (mModelFilter != nullptr) { mModelFilter->destroy(); }
    if (mImageFaceDetector != nullptr) { mImageFaceDetector->quitAndWait(); }
    if (mDownloadBuffer != 0) {
        glDeleteBuffers(1, &mDownloadBuffer);
        mDownloadBuffer = 0;
    }
    delete mMaskFilter;
    mMaskFilter = nullptr;
    delete mModelFilter;
    mModelFilter = nullptr;
    delete mBackgroundFilter;
    mBackgroundFilter = nullptr;
    delete mPlaceHolderFilter;
    mPlaceHolderFilter = nullptr;
    delete mFaceLiftFilter;
    mFaceLiftFilter = nullptr;
    delete mImageFaceDetector;
    mImageFaceDetector = nullptr;
}

void ImageRender::handleSurfaceChange(JNIEnv *env) {
    if (mDownloadBuffer != 0) { glDeleteBuffers(1, &mDownloadBuffer); }
    glGenBuffers(1, &mDownloadBuffer);

    int bufSize = mSurfaceWidth * mSurfaceHeight * 4;
    glBindBuffer(GL_PIXEL_PACK_BUFFER, mDownloadBuffer);
    glBufferData(GL_PIXEL_PACK_BUFFER, bufSize, nullptr, GL_STREAM_READ);

    glBindBuffer(GL_PIXEL_PACK_BUFFER, GL_NONE);
}

void ImageRender::handleDownloadRawPreview() {
    if (mImageFaceDetector != nullptr && mImageFaceDetector->isRunning()) {
        mBackgroundFilter->flip(false, true);
        mBackgroundFilter->onDraw(RENDER_NO_TEXTURE);

        glBindFramebuffer(GL_FRAMEBUFFER, mBackgroundFilter->getFrameBuffer());

        glBindBuffer(GL_PIXEL_PACK_BUFFER, mDownloadBuffer);
        glReadPixels(0, 0, mSurfaceWidth, mSurfaceHeight, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
        unsigned char* pixelData = nullptr;
        pixelData = static_cast<unsigned char *>(glMapBufferRange(GL_PIXEL_PACK_BUFFER, 0,
                                                                  mSurfaceWidth * mSurfaceHeight * 4,
                                                                  GL_MAP_READ_BIT));
        if (pixelData) {
            LogUtil::logI(TAG, {"handleDownloadRawPreview: get framebuffer data"});
            switch (mDownloadMode) {
                case render::DownloadMode::MODE_FACE_DETECT: {
                    mImageFaceDetector->enqueueImg(pixelData, mSurfaceWidth, mSurfaceHeight, 4, EventType::EVENT_FACE_LAND_MARK_TRACK);
                    break;
                }
                case render::DownloadMode::MODE_WRITE_PNG: {
                    mImageFaceDetector->enqueueImg(pixelData, mSurfaceWidth, mSurfaceHeight, 4, EventType::EVENT_WRITE_PNG);
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
    } else {
        LogUtil::logI(TAG, {"handleDownloadRawPreview: detector is not running, retry"});
        EventMessage msg(EventType::EVENT_FACE_TRACK_START, 500);
        enqueueMessage(msg);
    }
}

void ImageRender::notifyImageLandMarkTrackStart(JNIEnv *env) {
    jobject listener = GET_LISTENER;
    if (listener) {
        jclass listenerClass = env->GetObjectClass(listener);
        jmethodID methodId = env->GetMethodID(listenerClass, "onTrackImageLandMarkStart", "()V");
        env->CallVoidMethod(listener, methodId);
    }
}

void ImageRender::notifyImageLandMarkTrackFinish(JNIEnv *env) {
    jobject listener = GET_LISTENER;
    if (listener) {
        jclass listenerClass = env->GetObjectClass(listener);
        jmethodID methodId = env->GetMethodID(listenerClass, "onTrackImageLandMarkFinish", "()V");
        env->CallVoidMethod(listener, methodId);
    }
}

void ImageRender::setResource(JNIEnv *env, jobject bitmap) {
    if (mBackgroundFilter == nullptr) { mBackgroundFilter = new BackgroundFilter; }
    mBackgroundFilter->setOutputSize(mSurfaceWidth, mSurfaceHeight);
    mBackgroundFilter->setBitmap(env, bitmap);
    if (!mBackgroundFilter->initialized()) {
        std::shared_ptr<WorkTask> task = std::make_shared<FilterInitTask>();
        task->setObj(mBackgroundFilter);
        mWorkQueue->enqueue(task);
    }

    if (mMaskFilter == nullptr) { mMaskFilter = new MaskFilter; }
    mMaskFilter->setOutputSize(mSurfaceWidth, mSurfaceHeight);
    mMaskFilter->setSourceSize(mBackgroundFilter->getBitmapWidth(), mBackgroundFilter->getBitmapHeight());
    mMaskFilter->buildMask();
    if (!mMaskFilter->initialized()) {
        std::shared_ptr<WorkTask> task = std::make_shared<FilterInitTask>();
        task->setObj(mMaskFilter);
        mWorkQueue->enqueue(task);
    }
}

void ImageRender::trackFace(bool start) {
    if (mImageFaceDetector == nullptr) {
        throw EnvNotPreparedException{};
    }
    if (start) {
        mImageFaceDetector->execute(true);
        enqueueMessage(EventType::EVENT_FACE_TRACK_START);
    } else {
        mImageFaceDetector->execute(false);
    }
}

