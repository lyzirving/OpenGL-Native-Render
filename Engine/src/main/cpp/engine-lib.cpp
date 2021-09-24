//
// Created by liuyuzhou on 2021/9/7.
//
#include <jni.h>

#include "LogUtil.h"
#include "GlUtil.h"
#include "Render.h"
#include "Static.h"

#include "CamRender.h"

#define TAG "Engine-lib"

JNIEXPORT int JNICALL JNI_OnLoad(JavaVM *vm,void *reserved) {
    JNIEnv *env;
    if (vm->GetEnv((void **) &env,JNI_VERSION_1_6) != JNI_OK) {
        LogUtil::logE(TAG, {"JNI_OnLoad: failed to get env"});
        return JNI_ERR;
    }
    EngineUtil::init();
    if (!Render::registerSelf(env)) { return JNI_ERR; }
    if (!GlUtil::registerSelf(env)) { return JNI_ERR; }
    if (!CamRender::registerSelf(env)) { return JNI_ERR; }
    return JNI_VERSION_1_6;
}
