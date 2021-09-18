//
// Created by liuyuzhou on 2021/8/27.
//
#include "JniUtil.h"
#include "LogUtil.h"

#define TAG "JniUtil"

void JniUtil::detachThread(JavaVM *vm) { vm->DetachCurrentThread(); }

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

bool JniUtil::threadAttachJvm(JavaVM *vm, JNIEnv **ppEnv) {
    if (vm->GetEnv((void **) ppEnv, JNI_VERSION_1_6) == JNI_EDETACHED) {
        if (vm->AttachCurrentThread(ppEnv, nullptr) != 0) {
            return false;
        }
    }
    return true;
}

jobject JniUtil::removeListener(std::map<jlong, jobject> *pMap, jlong key) {
    jobject listener = findListener(pMap, key);
    if (listener != nullptr) {
        pMap->erase(key);
    }
    return listener;
}

