//
// Created by liuyuzhou on 2021/9/7.
//

#ifndef ENGINE_STATIC_H
#define ENGINE_STATIC_H

#include <pthread.h>
#include "GlUtil.h"
#include "BitmapUtil.h"

namespace EngineUtil {
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
