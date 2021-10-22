//
// Created by liuyuzhou on 2021/9/7.
//
#include "GlUtil.h"
#include "JniUtil.h"
#include "CamRender.h"
#include "ImageRender.h"
#include "LogUtil.h"

#define TAG "Engine-lib"

JNIEXPORT int JNICALL JNI_OnLoad(JavaVM *vm,void *reserved) {
    JNIEnv *env;
    if (vm->GetEnv((void **) &env,JNI_VERSION_1_6) != JNI_OK) {
        LogUtil::logE(TAG, {"JNI_OnLoad: failed to get env"});
        return JNI_ERR;
    }
    if (!GlUtil::registerSelf(env)) { return JNI_ERR; }
    GlUtil::init(env);
    JniUtil::self()->init(env);
    if (!BaseRender::registerSelf(env)) { return JNI_ERR; }
    if (!CamRender::registerSelf(env)) { return JNI_ERR; }
    if (!ImageRender::registerSelf(env)) { return JNI_ERR; }
    return JNI_VERSION_1_6;
}
