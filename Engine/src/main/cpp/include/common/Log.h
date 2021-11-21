//
// Created by lyzirving on 2021/11/19.
//
#ifndef ENGINE_LOG_H
#define ENGINE_LOG_H

#include <android/log.h>

#define ROOT_TAG "Engine_Native"

#define LOG_D(...) __android_log_print(ANDROID_LOG_DEBUG, ROOT_TAG, __VA_ARGS__)
#define LOG_I(...) __android_log_print(ANDROID_LOG_INFO, ROOT_TAG, __VA_ARGS__)
#define LOG_E(...) __android_log_print(ANDROID_LOG_ERROR, ROOT_TAG, __VA_ARGS__)

#endif //ENGINE_LOG_H
