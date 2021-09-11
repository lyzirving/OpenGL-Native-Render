//
// Created by liuyuzhou on 2021/9/7.
//

#ifndef ENGINE_JNIUTIL_H
#define ENGINE_JNIUTIL_H

#include <jni.h>
#include <map>

class JniUtil {
public:
    static void detachThread(JavaVM* vm);
    static jobject findListener(std::map<jlong, jobject>* pMap, jlong key);
    static jobject removeListener(std::map<jlong, jobject>* pMap, jlong key);
    static bool threadAttachJvm(JavaVM* vm, JNIEnv **ppEnv);
private:
};

#endif //ENGINE_JNIUTIL_H
