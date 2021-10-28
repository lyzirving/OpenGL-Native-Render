//
// Created by liuyuzhou on 2021/10/27.
//
#ifndef ENGINE_SHARERENDER_H
#define ENGINE_SHARERENDER_H

#include "BaseRender.h"

class ShareRender : public BaseRender {
public:
    static bool registerSelf(JNIEnv *env);

    void drawFrame() override;

    void drawShare(GLuint inputShareTexture) override;

    void render(JNIEnv *env) override;

    void trackFace(bool start) override;

protected:
    void handleOtherMessage(JNIEnv *env, const EventMessage &msg) override;

    void handlePreDraw(JNIEnv *env) override;

    void handlePostDraw(JNIEnv *env) override;

    void handleRenderEnvResume(JNIEnv *env) override;

    void handleSurfaceChange(JNIEnv *env) override;

    void handleEnvPrepare(JNIEnv *env) override;

    void handleRenderEnvPause(JNIEnv *env) override;

    void handleRenderEnvDestroy(JNIEnv *env) override;

};

#endif //ENGINE_SHARERENDER_H
