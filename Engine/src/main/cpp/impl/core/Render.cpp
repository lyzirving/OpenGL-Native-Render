//
// Created by liuyuzhou on 2021/8/27.
//
#include "Render.h"
#include "JniUtil.h"
#include "LogUtil.h"
#include "FilterFactory.h"
#include "HighlightShadowFilter.h"

#include <map>
#include <pthread.h>

#define TAG "Render"
#define JAVA_CLASS "com/render/engine/core/RenderEngine"

static JavaVM *sGlobJvm = nullptr;
static std::map<jlong, jobject> sGlobalAdapters;
static const int RENDER_NO_TEXTURE = 0;

Render::Render() {
    mEvtQueue = new EventQueue;
    mWorkQueue = new WorkQueue;
    mEglCore = new RenderEglBase;
}

Render::~Render() = default;

void *renderLoop(void *args) {
    auto *pRender = static_cast<Render *>(args);
    JNIEnv *env = nullptr;
    if (!JniUtil::threadAttachJvm(sGlobJvm, &env)) {
        LogUtil::logI(TAG, {"renderLoop: failed to attach thread to jvm"});
        return nullptr;
    }
    pRender->render(env);
    JniUtil::detachThread(sGlobJvm);
    return nullptr;
}

static jlong nConstruct(JNIEnv *env, jclass clazz) {
    if (sGlobJvm == nullptr) { env->GetJavaVM(&sGlobJvm); }
    return reinterpret_cast<jlong>(new Render);
}

static void nReleaseEnv(JNIEnv *env, jclass clazz, jlong ptr) {
    auto *pRender = reinterpret_cast<Render *>(ptr);
    pRender->enqueueMessage(EventType::EVENT_QUIT);
}

static void nRequestRenderEnv(JNIEnv *env, jclass clazz, jlong ptr) {
    auto *pRender = reinterpret_cast<Render *>(ptr);
    pRender->enqueueMessage(EventType::EVENT_DRAW);
}

static jboolean nIsEnvInit(JNIEnv *env, jclass clazz, jlong ptr) {
    auto *pRender = reinterpret_cast<Render *>(ptr);
    return pRender->initialized();
}

static void nSetEnvResource(JNIEnv *env, jclass clazz, jlong ptr, jobject bitmap) {
    auto *pRender = reinterpret_cast<Render *>(ptr);
    pRender->setResource(env, bitmap);
}

static void nEnvSurfaceCreate(JNIEnv *env, jclass clazz, jlong ptr, jobject surface, jobject adapter) {
    auto *pRender = reinterpret_cast<Render *>(ptr);
    auto gUtil = GlUtil::self();
    if (!gUtil->mapContains(&sGlobalAdapters, ptr)) { sGlobalAdapters.insert(std::pair<jlong, jobject>(ptr, env->NewGlobalRef(adapter))); }
    if (!pRender->initialized()) {
        ANativeWindow *nativeWindow = ANativeWindow_fromSurface(env, surface);
        pRender->setNativeWindow(nativeWindow);
        pthread_t thread;
        pthread_create(&thread, nullptr, renderLoop, pRender);
        pthread_setname_np(thread, "render-loop");
    }
}

static void nEnvSurfaceChange(JNIEnv *env, jclass clazz, jlong ptr, jint width, jint height) {
    auto *pRender = reinterpret_cast<Render *>(ptr);
    pRender->setSize(width, height);
    pRender->enqueueMessage(EventType::EVENT_SURFACE_CHANGE);
}

static jboolean nEnvAddBeautyFilter(JNIEnv *env, jclass clazz, jlong ptr, jstring filterType) {
    auto *pRender = reinterpret_cast<Render *>(ptr);
    const char* type = env->GetStringUTFChars(filterType, JNI_FALSE);
    bool res = pRender->addBeautyFilter(type);
    env->ReleaseStringUTFChars(filterType, type);
    return res;
}

static void nEnvAdjust(JNIEnv *env, jclass clazz, jlong ptr, jstring filterType, jint progress) {
    auto *pRender = reinterpret_cast<Render *>(ptr);
    const char* type = env->GetStringUTFChars(filterType, JNI_FALSE);
    pRender->adjust(type, progress);
    env->ReleaseStringUTFChars(filterType, type);
}

static void nEnvAdjustProp(JNIEnv *env, jclass clazz, jlong ptr, jstring filterType, jstring filterProp, jint progress) {
    auto *pRender = reinterpret_cast<Render *>(ptr);
    const char* type = env->GetStringUTFChars(filterType, JNI_FALSE);
    const char* prop = env->GetStringUTFChars(filterProp, JNI_FALSE);
    pRender->adjustProp(type, prop, progress);
    env->ReleaseStringUTFChars(filterType, type);
    env->ReleaseStringUTFChars(filterProp, prop);
}

static void nEnvOnPause(JNIEnv *env, jclass clazz, jlong ptr) {
    auto *pRender = reinterpret_cast<Render *>(ptr);
    pRender->enqueueMessage(EventType::EVENT_PAUSE);
}

static void nEnvOnResume(JNIEnv *env, jclass clazz, jlong ptr, jobject surface) {
    auto *pRender = reinterpret_cast<Render *>(ptr);
    ANativeWindow *nativeWindow = ANativeWindow_fromSurface(env, surface);
    pRender->setNativeWindow(nativeWindow);
    pRender->enqueueMessage(EventType::EVENT_RESUME);
}

static void nEnvClearBeautyFilter(JNIEnv *env, jclass clazz, jlong ptr) {
    auto *pRender = reinterpret_cast<Render *>(ptr);
    pRender->clearBeautyFilter();
}

static JNINativeMethod sJniMethods[] = {
        {
                "nCreate",           "()J",
                (void *) nConstruct
        },
        {
                "nRelease",          "(J)V",
                (void *) nReleaseEnv
        },
        {
                "nRequestRender",    "(J)V",
                (void *) nRequestRenderEnv
        },
        {
                "nIsEnvInitialized", "(J)Z",
                (void *) nIsEnvInit
        },
        {
                "nSetResource",    "(JLandroid/graphics/Bitmap;)V",
                (void *) nSetEnvResource
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
                "nAddBeautyFilter", "(JLjava/lang/String;)Z",
                (void *) nEnvAddBeautyFilter
        },
        {
                "nAdjust", "(JLjava/lang/String;I)V",
                (void *) nEnvAdjust
        },
        {
                "nAdjustProp", "(JLjava/lang/String;Ljava/lang/String;I)V",
                (void *) nEnvAdjustProp
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
                "nClearBeautyFilter", "(J)V",
                (void *) nEnvClearBeautyFilter
        }
};

bool Render::addBeautyFilter(const char *filterType) {
    std::shared_ptr<BaseFilter> filter;
    std::shared_ptr<FilterInitTask> task;
    if (filterType == nullptr || std::strlen(filterType) == 0) {
        LogUtil::logI(TAG, {"addBeautyFilter: filter type is empty"});
        goto fail;
    }
    if (mBeautyFilterGroup == nullptr) { mBeautyFilterGroup = new BaseFilterGroup; }
    if (!mBeautyFilterGroup->containsFilter(filterType)) {
        filter = FilterFactory::makeFilter(filterType);
        if (filter != nullptr) {
            mBeautyFilterGroup->addFilter(filterType, filter);
            mBeautyFilterGroup->setOutputSize(mWidth, mHeight);
            task = std::make_shared<FilterInitTask>();
            task->setObj(mBeautyFilterGroup);
            mWorkQueue->enqueue(task);
        } else {
            LogUtil::logI(TAG, {"addBeautyFilter: factory could not create filter ", filterType});
            goto fail;
        }
    } else {
        LogUtil::logI(TAG, {"addBeautyFilter: already contains filter ", filterType});
        goto fail;
    }
    return true;
    fail:
    return false;
}

void Render::adjust(const char *filterType, int progress) {
    bool optDone = false;
    if (mBeautyFilterGroup != nullptr && mBeautyFilterGroup->containsFilter(filterType)) {
        std::shared_ptr<BaseFilter> filter = mBeautyFilterGroup->getFilter(filterType);
        filter->adjust(progress);
        optDone = true;
    }
}

void Render::adjustProp(const char *filterType, const char *prop, int progress) {
    if (filterType == nullptr || std::strlen(filterType) == 0) {
        LogUtil::logI(TAG, {"adjustProp: filter type is invalid"});
        return;
    }
    if (prop == nullptr || std::strlen(prop) == 0) {
        LogUtil::logI(TAG, {"adjustProp: filter prop is invalid"});
        return;
    }
    if (std::strcmp(EngineUtil::FILTER_HIGHLIGHT_SHADOW, filterType) == 0) {
        if (mBeautyFilterGroup != nullptr && mBeautyFilterGroup->containsFilter(filterType)) {
            std::shared_ptr<BaseFilter> tmp = mBeautyFilterGroup->getFilter(filterType);
            //do not delete filter pointer here, it should be kept alive in map;
            BaseFilter* baseFilter = tmp.get();
            auto* target = dynamic_cast<HighlightShadowFilter *>(baseFilter);
            if (std::strcmp(EngineUtil::FILTER_PROP_HIGHLIGHT, prop) == 0) {
                target->adjustHighlight(progress);
            } else if (std::strcmp(EngineUtil::FILTER_PROP_SHADOW, prop) == 0) {
                target->adjustShadow(progress);
            }
        }
    } else {
        LogUtil::logI(TAG, {"adjustProp: unknown filter ", filterType});
    }
}

void Render::clearBeautyFilter() {
    if (mBeautyFilterGroup != nullptr) {
        std::shared_ptr<FilterDestroyTask> task = std::make_shared<FilterDestroyTask>();
        task->setObj(mBeautyFilterGroup);
        mWorkQueue->enqueue(task);
    }
}

void Render::drawFrame() {
    LogUtil::logI(TAG, {"drawFrame"});
    while (!mWorkQueue->empty()) {
        std::shared_ptr<WorkTask> task = std::make_shared<WorkTask>();
        if (mWorkQueue->dequeue(task)) { task->run(); }
    }
    if (mBackgroundFilter != nullptr) {
        //background filter has content texture on its own, so you don't need to pass texture to it;
        int drawCount = 0;
        int lastTexture = mBackgroundFilter->onDraw(RENDER_NO_TEXTURE);
        drawCount++;
        if (mBeautyFilterGroup != nullptr) {
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
    if (!mEglCore->swapBuffer()) {
        LogUtil::logI(TAG, {"drawFrame: failed to swap buffer"});
    }
}

void Render::deleteFilters() {
    LogUtil::logI(TAG, {"deleteFilters"});
    if (mBeautyFilterGroup != nullptr) {
        mBeautyFilterGroup->destroy();
        delete mBeautyFilterGroup;
        mBeautyFilterGroup = nullptr;
    }
    if (mMaskFilter != nullptr) {
        mMaskFilter->destroy();
        delete mMaskFilter;
        mMaskFilter = nullptr;
    }
    if (mBackgroundFilter != nullptr) {
        mBackgroundFilter->destroy();
        delete mBackgroundFilter;
        mBackgroundFilter = nullptr;
    }
    if (mScreenFilter != nullptr) {
        mScreenFilter->destroy();
        delete mScreenFilter;
        mScreenFilter = nullptr;
    }
}

void Render::enqueueMessage(EventType what) {
    mEvtQueue->enqueueMessage(new EventMessage(what));
}

bool Render::initialized() {
    return mStatus >= RenderStatus::STATUS_PREPARE && mStatus <= RenderStatus::STATUS_PAUSE;
}

void Render::notifyEnvPrepare(JNIEnv *env, jobject listener) {
    jclass listenerClass = env->GetObjectClass(listener);
    jmethodID methodId = env->GetMethodID(listenerClass, "onRenderEnvPrepare", "()V");
    env->CallVoidMethod(listener, methodId);
}

void Render::notifyEnvRelease(JNIEnv *env, jobject listener) {
    jclass listenerClass = env->GetObjectClass(listener);
    jmethodID methodId = env->GetMethodID(listenerClass, "onRenderEnvRelease", "()V");
    env->CallVoidMethod(listener, methodId);
}

void Render::render(JNIEnv *env) {
    jobject listener = JniUtil::findListener(&sGlobalAdapters, reinterpret_cast<jlong>(this));
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
                break;
            }
            case EventType::EVENT_PAUSE: {
                LogUtil::logI(TAG, {"render: handle message pause"});
                mStatus = RenderStatus::STATUS_PAUSE;
                if (mBeautyFilterGroup != nullptr) { mBeautyFilterGroup->onPause(); }
                if (mBackgroundFilter != nullptr) { mBackgroundFilter->onPause(); }
                if (mScreenFilter != nullptr) { mScreenFilter->onPause(); }
                if (mMaskFilter != nullptr) { mMaskFilter->onPause(); }
                if (mEglCore != nullptr) {
                    mEglCore->release();
                }
                break;
            }
            case EventType::EVENT_RESUME: {
                LogUtil::logI(TAG, {"render: handle message resume"});
                mStatus = RenderStatus::STATUS_RUN;
                if (mEglCore != nullptr) {
                    if (!mEglCore->initEglEnv()) {
                        goto quit;
                    }
                }
                break;
            }
            case EventType::EVENT_DRAW: {
                LogUtil::logI(TAG, {"render: handle message draw"});
                mStatus = RenderStatus::STATUS_RUN;
                drawFrame();
                break;
            }
            case EventType::EVENT_QUIT: {
                LogUtil::logI(TAG, {"render: handle message quit"});
                mStatus = RenderStatus::STATUS_DESTROY;
                deleteFilters();
                goto quit;
            }
            case EventType::EVENT_IDLE:
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

void Render::release(JNIEnv *env) {
    jobject adapter = JniUtil::removeListener(&sGlobalAdapters, reinterpret_cast<jlong>(this));
    if (adapter != nullptr) { env->DeleteGlobalRef(adapter); }
    if (mEglCore != nullptr) { mEglCore->release(); }
    if (mEvtQueue != nullptr) { delete mEvtQueue; }
    if (mWorkQueue != nullptr) { delete mWorkQueue; }
    mEglCore = nullptr;
    mEvtQueue = nullptr;
}

bool Render::registerSelf(JNIEnv *env) {
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
    return true;
}

void Render::setResource(JNIEnv *env, jobject bitmap) {
    if (mBackgroundFilter == nullptr) { mBackgroundFilter = new BackgroundFilter; }
    mBackgroundFilter->setOutputSize(mWidth, mHeight);
    mBackgroundFilter->setBitmap(env, bitmap);
    if (!mBackgroundFilter->initialized()) {
        std::shared_ptr<FilterInitTask> task = std::make_shared<FilterInitTask>();
        task->setObj(mBackgroundFilter);
        mWorkQueue->enqueue(task);
    }
    if (mMaskFilter == nullptr) { mMaskFilter = new MaskFilter; }
    mMaskFilter->setOutputSize(mWidth, mHeight);
    mMaskFilter->setSourceSize(mBackgroundFilter->getBitmapWidth(), mBackgroundFilter->getBitmapHeight());
    mMaskFilter->buildMask();
    if (!mMaskFilter->initialized()) {
        std::shared_ptr<FilterInitTask> task = std::make_shared<FilterInitTask>();
        task->setObj(mMaskFilter);
        mWorkQueue->enqueue(task);
    }
    if (mScreenFilter == nullptr) { mScreenFilter = new ScreenFilter; }
    mScreenFilter->setOutputSize(mWidth, mHeight);
    if (!mScreenFilter->initialized()) {
        std::shared_ptr<FilterInitTask> task = std::make_shared<FilterInitTask>();
        task->setObj(mScreenFilter);
        mWorkQueue->enqueue(task);
    }
    if (mBeautyFilterGroup != nullptr) {
        mBeautyFilterGroup->setOutputSize(mWidth, mHeight);
        std::shared_ptr<FilterInitTask> task = std::make_shared<FilterInitTask>();
        task->setObj(mBeautyFilterGroup);
        mWorkQueue->enqueue(task);
    }
}

void Render::setSize(GLint width, GLint height) {
    mWidth = width;
    mHeight = height;
}

void Render::setNativeWindow(ANativeWindow *window) {
    mEglCore->setNativeWindow(window);
}

