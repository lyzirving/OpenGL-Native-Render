//
// Created by liuyuzhou on 2021/9/7.
//
#ifndef ENGINE_JNIUTIL_H
#define ENGINE_JNIUTIL_H

#include <jni.h>
#include <map>

class JniUtil {
public:
    static JniUtil* self();

    ~JniUtil();

    bool attachJvm(JNIEnv **ppEnv);
    void detachThread();
    void init(JNIEnv* env);
    jobject find(std::map<std::string, jobject>* pMap, const std::string& key);
    jobject findListener(std::map<jlong, jobject>* pMap, jlong key);
    jobject removeListener(std::map<jlong, jobject>* pMap, jlong key);
private:
    JniUtil();
};

#endif //ENGINE_JNIUTIL_H
