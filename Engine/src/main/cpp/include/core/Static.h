//
// Created by liuyuzhou on 2021/9/7.
//

#ifndef ENGINE_STATIC_H
#define ENGINE_STATIC_H

#include <pthread.h>
#include "GlUtil.h"
#include "BitmapUtil.h"

namespace EngineUtil {
    static const char* FILTER_CONTRAST = "CONTRAST";
    static const char* FILTER_SHARPEN = "SHARPEN";
    static const char* FILTER_SATURATION = "SATURATION";
    static const char* FILTER_EXPOSURE = "EXPOSURE";
    static const char* FILTER_HIGHLIGHT_SHADOW = "HIGHLIGHT_SHADOW";

    static const char* FILTER_PROP_HIGHLIGHT = "HIGHLIGHT";
    static const char* FILTER_PROP_SHADOW = "SHADOW";

    static BitmapUtil *gBitmapUtl = nullptr;
    static GlUtil *gGlUtil = nullptr;

    static pthread_mutex_t gMutex;

    static void destroy() {
        pthread_mutex_destroy(&gMutex);
    }

    static void init() {
        pthread_mutex_init(&gMutex, nullptr);
    }

    static void lock() {
        pthread_mutex_lock(&gMutex);
    }

    static void unlock() {
        pthread_mutex_unlock(&gMutex);
    }
}

#endif //ENGINE_STATIC_H
