//
// Created by lyzirving on 2021/9/25.
//
#ifndef ENGINE_IMAGERENDER_H
#define ENGINE_IMAGERENDER_H

#include "BaseRender.h"
#include "BackgroundFilter.h"
#include "MaskFilter.h"

class ImageRender : public BaseRender{
public:
    static bool registerSelf(JNIEnv *env);

    void drawFrame() override;
    void setResource(JNIEnv* env, jobject bitmap);

protected:

    void handleEnvPrepare(JNIEnv *env) override;
    void handleOtherMessage(JNIEnv *env, EventType what) override;
    void handlePreDraw(JNIEnv *env) override;
    void handlePostDraw(JNIEnv *env) override;
    void handleRenderEnvPause(JNIEnv *env) override;
    void handleRenderEnvResume(JNIEnv *env) override;
    void handleRenderEnvDestroy(JNIEnv *env) override;
    void handleSurfaceChange(JNIEnv *env) override;

private:
    MaskFilter* mMaskFilter{nullptr};
    BackgroundFilter* mBackgroundFilter{nullptr};
};

#endif //ENGINE_IMAGERENDER_H
