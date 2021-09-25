//
// Created by liuyuzhou on 2021/9/24.
//
#include "BaseRender.h"
#include "FilterFactory.h"
#include "JniUtil.h"
#include "LogUtil.h"

#include "HighlightShadowFilter.h"
#include "GaussianFilter.h"

#define TAG "BaseRender"
#define JAVA_CLASS_BASE_RENDER "com/render/engine/core/BaseRenderEngine"

BaseRender::BaseRender() {
    mWorkQueue = new WorkQueue;
    mEvtQueue = new EventQueue;
    mEglCore = new RenderEglBase;
}

BaseRender::~BaseRender() = default;

void *envRenderLoop(void *args) {
    auto *pRender = static_cast<BaseRender *>(args);
    JNIEnv *env = nullptr;
    if (!JniUtil::threadAttachJvm(render::gJvm, &env)) {
        LogUtil::logI(TAG, {"renderLoop: failed to attach thread to jvm"});
        return nullptr;
    }
    pRender->render(env);
    JniUtil::detachThread(render::gJvm);
    return nullptr;
}

static void nEnvAdjust(JNIEnv *env, jclass clazz, jlong ptr, jstring filterType, jint progress) {
    auto *pRender = reinterpret_cast<BaseRender *>(ptr);
    const char* type = env->GetStringUTFChars(filterType, JNI_FALSE);
    pRender->adjust(type, progress);
    env->ReleaseStringUTFChars(filterType, type);
}

static void nEnvAdjustProperty(JNIEnv *env, jclass clazz, jlong ptr, jstring filterType, jstring property, jint progress) {
    auto *pRender = reinterpret_cast<BaseRender *>(ptr);
    const char* type = env->GetStringUTFChars(filterType, JNI_FALSE);
    const char* prop = env->GetStringUTFChars(property, JNI_FALSE);
    pRender->adjustProperty(type, prop, progress);
    env->ReleaseStringUTFChars(filterType, type);
    env->ReleaseStringUTFChars(property, prop);
}

static jboolean nEnvAddBeautyFilter(JNIEnv *env, jclass clazz, jlong ptr, jstring filterType, jboolean commit) {
    auto *pRender = reinterpret_cast<BaseRender *>(ptr);
    const char* type = env->GetStringUTFChars(filterType, JNI_FALSE);
    bool res = pRender->addBeautyFilter(type, commit);
    env->ReleaseStringUTFChars(filterType, type);
    return res;
}

static void nEnvClearBeautyFilter(JNIEnv *env, jclass clazz, jlong ptr) {
    auto *pRender = reinterpret_cast<BaseRender *>(ptr);
    pRender->clearBeautyFilter();
}

static jboolean nEnvInitialized(JNIEnv *env, jclass clazz, jlong ptr) {
    auto *pRender = reinterpret_cast<BaseRender *>(ptr);
    return pRender->initialized();
}

static void nEnvOnPause(JNIEnv *env, jclass clazz, jlong ptr) {
    auto *pRender = reinterpret_cast<BaseRender *>(ptr);
    pRender->enqueueMessage(EventType::EVENT_PAUSE);
}

static void nEnvOnResume(JNIEnv *env, jclass clazz, jlong ptr, jobject surface) {
    auto *pRender = reinterpret_cast<BaseRender *>(ptr);
    ANativeWindow *nativeWindow = ANativeWindow_fromSurface(env, surface);
    pRender->setNativeWindow(nativeWindow);
    pRender->enqueueMessage(EventType::EVENT_RESUME);
}

static void nEnvRequestRender(JNIEnv *env, jclass clazz, jlong ptr) {
    auto *pRender = reinterpret_cast<BaseRender *>(ptr);
    pRender->enqueueMessage(EventType::EVENT_DRAW);
}

static void nEnvRelease(JNIEnv *env, jclass clazz, jlong ptr) {
    auto *pRender = reinterpret_cast<BaseRender *>(ptr);
    pRender->enqueueMessage(EventType::EVENT_QUIT);
}

static void nEnvSurfaceCreate(JNIEnv *env, jclass clazz, jlong ptr, jobject surface, jobject adapter) {
    auto *pRender = reinterpret_cast<BaseRender*>(ptr);
    pRender->setJavaListener(env, adapter);
    render::getJvm(env);
    if (!pRender->initialized()) {
        ANativeWindow *nativeWindow = ANativeWindow_fromSurface(env, surface);
        pRender->setNativeWindow(nativeWindow);
        pthread_t thread;
        pthread_create(&thread, nullptr, envRenderLoop, pRender);
    }
}

static void nEnvSurfaceChange(JNIEnv *env, jclass clazz, jlong ptr, jint width, jint height) {
    auto *pRender = reinterpret_cast<BaseRender *>(ptr);
    pRender->setSurfaceSize(width, height);
    pRender->enqueueMessage(EventType::EVENT_SURFACE_CHANGE);
}

static JNINativeMethod sJniMethods[] = {
        {
                "nAddBeautyFilter", "(JLjava/lang/String;Z)Z",
                (void *) nEnvAddBeautyFilter
        },
        {
                "nAdjust", "(JLjava/lang/String;I)V",
                (void *) nEnvAdjust
        },
        {
                "nAdjustProperty", "(JLjava/lang/String;Ljava/lang/String;I)V",
                (void *) nEnvAdjustProperty
        },
        {
                "nClearBeautyFilter", "(J)V",
                (void *) nEnvClearBeautyFilter
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
};

bool BaseRender::registerSelf(JNIEnv *env) {
    int count = sizeof(sJniMethods) / sizeof(sJniMethods[0]);
    jclass javaClass = env->FindClass(JAVA_CLASS_BASE_RENDER);
    if (!javaClass) {
        LogUtil::logE(TAG, {"registerSelf: failed to find class ", JAVA_CLASS_BASE_RENDER});
        return false;
    }
    if (env->RegisterNatives(javaClass, sJniMethods, count) < 0) {
        LogUtil::logE(TAG, {"registerSelf: failed to register native methods ", JAVA_CLASS_BASE_RENDER});
        return false;
    }
    return true;
}

void BaseRender::enqueueMessage(EventType what) {
    mEvtQueue->enqueueMessage(new EventMessage(what));
}

void BaseRender::adjust(const char *filterType, int progress) {
    if (mBeautyFilterGroup != nullptr && mBeautyFilterGroup->containsFilter(filterType)) {
        std::shared_ptr<BaseFilter> filter = mBeautyFilterGroup->getFilter(filterType);
        filter->adjust(progress);
    }
}

void BaseRender::adjustProperty(const char *filterType, const char *property, int progress) {
    if (filterType == nullptr || std::strlen(filterType) == 0) {
        LogUtil::logI(TAG, {"adjustProperty: filter type is invalid"});
        return;
    }
    if (property == nullptr || std::strlen(property) == 0) {
        LogUtil::logI(TAG, {"adjustProperty: filter prop is invalid"});
        return;
    }
    if (std::strcmp(render::FILTER_HIGHLIGHT_SHADOW, filterType) == 0) {
        if (mBeautyFilterGroup != nullptr && mBeautyFilterGroup->containsFilter(filterType)) {
            std::shared_ptr<BaseFilter> tmp = mBeautyFilterGroup->getFilter(filterType);
            //do not delete filter pointer here, it should be kept alive in map;
            BaseFilter* baseFilter = tmp.get();
            auto* target = dynamic_cast<HighlightShadowFilter *>(baseFilter);
            if (std::strcmp(render::FILTER_PROP_HIGHLIGHT, property) == 0) {
                target->adjustHighlight(progress);
            } else if (std::strcmp(render::FILTER_PROP_SHADOW, property) == 0) {
                target->adjustShadow(progress);
            }
        }
    } else if (std::strcmp(render::FILTER_GAUSSIAN, filterType) == 0) {
        if (mBeautyFilterGroup != nullptr && mBeautyFilterGroup->containsFilter(filterType)) {
            std::shared_ptr<BaseFilter> tmp = mBeautyFilterGroup->getFilter(filterType);
            //do not delete filter pointer here, it should be kept alive in map;
            BaseFilter* baseFilter = tmp.get();
            auto* target = dynamic_cast<GaussianFilter *>(baseFilter);
            if (std::strcmp(render::FILTER_PROP_HOR_GAUSSIAN, property) == 0) {
                target->adjustHorBlur(progress);
            } else if (std::strcmp(render::FILTER_PROP_VER_GAUSSIAN, property) == 0) {
                target->adjustVerBlur(progress);
            }
        }
    } else {
        LogUtil::logI(TAG, {"adjustProperty: unknown filter ", filterType});
    }
}

bool BaseRender::addBeautyFilter(const char *filterType, bool commit) {
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
            LogUtil::logI(TAG, {"addBeautyFilter: ", filterType});
            mBeautyFilterGroup->addFilter(filterType, filter);
            mBeautyFilterGroup->setOutputSize(mSurfaceWidth, mSurfaceHeight);
            if (commit) {
                task = std::make_shared<FilterInitTask>();
                task->setObj(mBeautyFilterGroup);
                mWorkQueue->enqueue(task);
            }
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

void BaseRender::clearBeautyFilter() {
    if (mBeautyFilterGroup != nullptr) {
        std::shared_ptr<FilterDestroyTask> task = std::make_shared<FilterDestroyTask>();
        task->setObj(mBeautyFilterGroup);
        mWorkQueue->enqueue(task);
    }
}

bool BaseRender::initialized() {
    return mStatus >= render::Status::STATUS_PREPARE && mStatus <= render::Status::STATUS_PAUSE;
}

void BaseRender::notifyEnvPrepare(JNIEnv *env, jobject listener) {
    if (listener != nullptr) {
        jclass listenerClass = env->GetObjectClass(listener);
        jmethodID methodId = env->GetMethodID(listenerClass, "onRenderEnvPrepare", "()V");
        env->CallVoidMethod(listener, methodId);
    }
}

void BaseRender::notifyEnvRelease(JNIEnv *env, jobject listener) {
    if (listener != nullptr) {
        jclass listenerClass = env->GetObjectClass(listener);
        jmethodID methodId = env->GetMethodID(listenerClass, "onRenderEnvRelease", "()V");
        env->CallVoidMethod(listener, methodId);
    }
}

void BaseRender::render(JNIEnv *env) {
    if (!mEglCore->initEglEnv()) { goto quit; }
    mStatus = render::Status::STATUS_PREPARE;
    notifyEnvPrepare(env, mJavaListener);
    for (;;) {
        EventMessage &&message = mEvtQueue->dequeueMessage();
        switch (message.what) {
            case EventType::EVENT_SURFACE_CHANGE: {
                LogUtil::logI(TAG, {"render: handle message surface change"});
                mStatus = render::Status::STATUS_RUN;
                surfaceChange();
                handleSurfaceChange(env);
                break;
            }
            case EventType::EVENT_PAUSE: {
                LogUtil::logI(TAG, {"render: handle message pause"});
                mStatus = render::Status::STATUS_PAUSE;
                renderEnvPause();
                handleRenderEnvPause(env);
                break;
            }
            case EventType::EVENT_RESUME: {
                LogUtil::logI(TAG, {"render: handle message resume"});
                mStatus = render::Status::STATUS_RUN;
                bool success = renderEnvResume();
                if (success) {
                    notifyEnvPrepare(env, mJavaListener);
                    handleRenderEnvResume(env);
                } else {
                    goto quit;
                }
                break;
            }
            case EventType::EVENT_DRAW: {
                bool valid = mEglCore != nullptr && mEglCore->valid();
                if (valid) {
                    mStatus = render::Status::STATUS_RUN;
                    runTaskPreDraw();
                    handlePreDraw(env);
                    drawFrame();
                    handlePostDraw(env);
                } else {
                    LogUtil::logI(TAG, {"render: handle message draw, env is not valid"});
                }
                break;
            }
            case EventType::EVENT_QUIT: {
                LogUtil::logI(TAG, {"render: handle message quit"});
                mStatus = render::Status::STATUS_DESTROY;
                renderEnvDestroy();
                handleRenderEnvDestroy(env);
                goto quit;
            }
            default: {
                LogUtil::logI(TAG, {"render: handle default msg"});
                handleOtherMessage(env, message.what);
                break;
            }
        }
    }
    quit:
    LogUtil::logI(TAG, {"render: quit render-loop"});
    notifyEnvRelease(env, mJavaListener);
    release(env);
    delete this;
}

void BaseRender::release(JNIEnv *env) {
    if (mJavaListener != nullptr) { env->DeleteGlobalRef(mJavaListener); }
    delete mEglCore;
    delete mEvtQueue;
    delete mWorkQueue;
    mJavaListener = nullptr;
    mWorkQueue = nullptr;
    mEglCore = nullptr;
    mEvtQueue = nullptr;
}

void BaseRender::renderEnvPause() {
    mEvtQueue->clear();
    mWorkQueue->clear();
    mEglCore->release();
}

bool BaseRender::renderEnvResume() {
    return mEglCore->initEglEnv();
}

void BaseRender::renderEnvDestroy() {
    mEvtQueue->clear();
    mWorkQueue->clear();
    mEglCore->release();
}

void BaseRender::runTaskPreDraw() {
    while (!mWorkQueue->empty()) {
        std::shared_ptr<WorkTask> task = std::make_shared<WorkTask>();
        if (mWorkQueue->dequeue(task)) { task->run(); }
    }
}

void BaseRender::setJavaListener(JNIEnv *env, jobject listener) {
    if (mJavaListener != nullptr) { env->DeleteGlobalRef(mJavaListener); }
    if (listener != nullptr) { mJavaListener = env->NewGlobalRef(listener); }
}

void BaseRender::setNativeWindow(ANativeWindow *window) {
    mEglCore->setNativeWindow(window);
}

void BaseRender::setSurfaceSize(GLint surfaceWidth, GLint surfaceHeight) {
    mSurfaceWidth = surfaceWidth;
    mSurfaceHeight = surfaceHeight;
}

void BaseRender::surfaceChange() {
    glViewport(0, 0, mSurfaceWidth, mSurfaceHeight);
    if (!mEglCore->swapBuffer()) { LogUtil::logI(TAG, {"surfaceChange: failed to swap buffer"}); }
}