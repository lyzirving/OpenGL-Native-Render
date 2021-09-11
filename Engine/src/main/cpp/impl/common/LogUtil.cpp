#include <android/log.h>
#include "LogUtil.h"

#define TAG "Engine_Native"

void LogUtil::logD(std::string tag, std::initializer_list<std::string> msg) {
    std::string result = std::move(tag);
    result.append("_");
    for (const auto & tmp : msg) {
        result.append(tmp);
    }
    __android_log_print(ANDROID_LOG_DEBUG, TAG, "%s", result.c_str());
}

void LogUtil::logI(std::string tag, std::initializer_list<std::string> msg) {
    std::string result = std::move(tag);
    result.append("_");
    for (const auto & tmp : msg) {
        result.append(tmp);
    }
    __android_log_print(ANDROID_LOG_INFO, TAG, "%s", result.c_str());
}

void LogUtil::logE(std::string tag, std::initializer_list<std::string> msg) {
    std::string result = std::move(tag);
    result.append("_");
    for (const auto & tmp : msg) {
        result.append(tmp);
    }
    __android_log_print(ANDROID_LOG_ERROR, TAG, "%s", result.c_str());
}

