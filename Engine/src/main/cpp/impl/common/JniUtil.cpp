//
// Created by liuyuzhou on 2021/8/27.
//
#include "JniUtil.h"
#include "LogUtil.h"

#include <thread>


#define TAG "JniUtil"

static JniUtil *gJniUtil = nullptr;
static std::mutex gMutex;
static JavaVM *gJvm{nullptr};

JniUtil::JniUtil() {
    LogUtil::logI(TAG, {"construct"});
}

JniUtil::~JniUtil() {
    LogUtil::logI(TAG, {"deconstruct"});
}

JniUtil *JniUtil::self() {
    {
        std::lock_guard<std::mutex> lockGuard(gMutex);
        if (gJniUtil == nullptr) { gJniUtil = new JniUtil; }
    }
    return gJniUtil;
}

bool JniUtil::attachJvm(JNIEnv **ppEnv) {
    if (gJvm->GetEnv((void **) ppEnv, JNI_VERSION_1_6) == JNI_EDETACHED) {
        if (gJvm->AttachCurrentThread(ppEnv, nullptr) != 0) {
            return false;
        }
    }
    return true;
}

void JniUtil::detachThread() { gJvm->DetachCurrentThread(); }

void JniUtil::init(JNIEnv *env) {
    env->GetJavaVM(&gJvm);
}

jobject JniUtil::find(std::map<std::string, jobject> *pMap, const std::string &key) {
    auto iterator = pMap->find(key);
    if (iterator == pMap->end()) {
        return nullptr;
    } else {
        return iterator->second;
    }
}

jobject JniUtil::findListener(std::map<jlong, jobject> *pMap, jlong key) {
    auto iterator = pMap->find(key);
    if (iterator == pMap->end()) {
        LogUtil::logE(TAG, {"findListener: failed to find listener"});
        return nullptr;
    }
    return iterator->second;
}

jobject JniUtil::removeListener(std::map<jlong, jobject> *pMap, jlong key) {
    jobject listener = findListener(pMap, key);
    if (listener != nullptr) {
        pMap->erase(key);
    }
    return listener;
}

