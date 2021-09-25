//
// Created by liuyuzhou on 2021/9/24.
//
#ifndef ENGINE_CAMRENDER_H
#define ENGINE_CAMRENDER_H

#include "BaseRender.h"
#include "OesFilter.h"

class CamRender : public BaseRender {
public:

    static bool registerSelf(JNIEnv *env);

    void drawFrame() override;
    void setCameraMetadata(JNIEnv* env, jobject data);
    void setPreview(GLint width, GLint height);
    void setSurfaceTexture(JNIEnv* env, jobject surfaceTexture);

protected:

    void handleOtherMessage(JNIEnv* env, EventType what) override;
    void handlePreDraw(JNIEnv *env) override;
    void handlePostDraw(JNIEnv *env) override;
    void handleRenderEnvPause(JNIEnv *env) override;
    void handleRenderEnvResume(JNIEnv *env) override;
    void handleRenderEnvDestroy(JNIEnv *env) override;
    void handleSurfaceChange(JNIEnv *env) override;

    void buildOesTexture();
    void buildCameraTransMatrix();
    void destroy(JNIEnv* env);
    void notifyEnvOesTextureCreate(JNIEnv* env, jobject listener, int oesTexture);
    void pause(JNIEnv* env);
    void updateTexImg(JNIEnv* env);

private:
    GLint mPreviewWidth{0};
    GLint mPreviewHeight{0};
    GLuint mOesTexture{0};
    GLfloat *mCamMatrix{nullptr};

    jobject mSurfaceTexture{nullptr};
    render::CameraMetaData* mCamMetaData{nullptr};

    OesFilter* mOesFilter{nullptr};
};

#endif //ENGINE_CAMRENDER_H
