//
// Created by lyzirving on 2021/9/25.
//
#include "ImageRender.h"
#include "LogUtil.h"

#define TAG "ImageRender"
#define JAVA_CLASS_IMAGE_RENDER "com/render/engine/img/ImageRenderEngine"
static const int RENDER_NO_TEXTURE = 0;

static jlong nEnvCreate(JNIEnv *env, jclass clazz) {
    return reinterpret_cast<jlong>(new ImageRender);
}

static void nEnvSetResource(JNIEnv *env, jclass clazz, jlong ptr, jobject bitmap) {
    auto *pRender = reinterpret_cast<ImageRender *>(ptr);
    pRender->setResource(env, bitmap);
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

void ImageRender::drawFrame() {
    LogUtil::logI(TAG, {"drawFrame"});
    if (mBackgroundFilter != nullptr) {
        //background filter has content texture on its own, so you don't need to pass texture to it;
        int drawCount = 0;
        int lastTexture = mBackgroundFilter->onDraw(RENDER_NO_TEXTURE);
        drawCount++;
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
    }
}

void ImageRender::handleEnvPrepare(JNIEnv *env) {}

void ImageRender::handleOtherMessage(JNIEnv *env, EventType what) {}

void ImageRender::handlePreDraw(JNIEnv *env) {
    glClearColor(1.0, 1.0, 1.0, 1.0);
    glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
}

void ImageRender::handlePostDraw(JNIEnv *env) {
    if (!mEglCore->swapBuffer()) {
        LogUtil::logI(TAG, {"drawFrame: failed to swap buffer"});
    }
}

void ImageRender::handleRenderEnvPause(JNIEnv *env) {
    if (mBeautyFilterGroup != nullptr) { mBeautyFilterGroup->onPause(); }
    if (mBackgroundFilter != nullptr) { mBackgroundFilter->onPause(); }
    if (mScreenFilter != nullptr) { mScreenFilter->onPause(); }
    if (mMaskFilter != nullptr) { mMaskFilter->onPause(); }
}

void ImageRender::handleRenderEnvResume(JNIEnv *env) {}

void ImageRender::handleRenderEnvDestroy(JNIEnv *env) {
    if (mBeautyFilterGroup != nullptr) { mBeautyFilterGroup->destroy(); }
    if (mMaskFilter != nullptr) { mMaskFilter->destroy(); }
    if (mBackgroundFilter != nullptr) { mBackgroundFilter->destroy(); }
    if (mScreenFilter != nullptr) { mScreenFilter->destroy(); }
    delete mBeautyFilterGroup;
    mBeautyFilterGroup = nullptr;
    delete mMaskFilter;
    mMaskFilter = nullptr;
    delete mBackgroundFilter;
    mBackgroundFilter = nullptr;
    delete mScreenFilter;
    mScreenFilter = nullptr;
}

void ImageRender::handleSurfaceChange(JNIEnv *env) {}

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

    if (mScreenFilter == nullptr) { mScreenFilter = new ScreenFilter; }
    mScreenFilter->setOutputSize(mSurfaceWidth, mSurfaceHeight);
    if (!mScreenFilter->initialized()) {
        std::shared_ptr<WorkTask> task = std::make_shared<FilterInitTask>();
        task->setObj(mScreenFilter);
        mWorkQueue->enqueue(task);
    }

    if (mBeautyFilterGroup != nullptr) {
        mBeautyFilterGroup->setOutputSize(mSurfaceWidth, mSurfaceHeight);
        std::shared_ptr<WorkTask> task = std::make_shared<FilterInitTask>();
        task->setObj(mBeautyFilterGroup);
        mWorkQueue->enqueue(task);
    }
}

