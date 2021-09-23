//
// Created by liuyuzhou on 2021/9/17.
//
#ifndef ENGINE_CAMERARENDER_H
#define ENGINE_CAMERARENDER_H

#include "Static.h"
#include "RenderEglBase.h"
#include "EventQueue.h"
#include "WorkQueue.h"
#include "ScreenFilter.h"
#include "OesFilter.h"

class CameraRender {
public:

    CameraRender();
    static bool registerSelf(JNIEnv *env);
    void buildOesTexture();
    void buildCameraTransMatrix();
    void drawFrame();
    void enqueueMessage(EventType what);
    bool initialized();
    void notifyEnvOesTextureCreate(JNIEnv* env, jobject listener, int oesTexture);
    void notifyEnvPrepare(JNIEnv* env, jobject listener);
    void notifyEnvRelease(JNIEnv* env, jobject listener);
    void render(JNIEnv* env);
    void release(JNIEnv* env);
    void releaseBeforeEnvDestroy(JNIEnv* env);
    void runBeforeDraw();
    void setPreview(GLint width, GLint height);
    void setSize(GLint width, GLint height);
    void setNativeWindow(ANativeWindow* window);
    void setSurfaceTexture(JNIEnv* env, jobject surfaceTexture);
    void setCameraMetadata(JNIEnv* env, jobject data);
    void updateTexImg(JNIEnv* env);

private:
    RenderEglBase* mEglCore;
    EventQueue* mEvtQueue;
    WorkQueue* mWorkQueue;
    GLint mWidth{0};
    GLint mHeight{0};
    GLint mPreviewWidth{0};
    GLint mPreviewHeight{0};
    GLuint mOesTexture{0};
    RenderStatus mStatus = RenderStatus::STATUS_IDLE;
    jobject mSurfaceTexture{nullptr};

    EngineUtil::CameraMetaData* mMetadata{nullptr};
    ScreenFilter* mScreenFilter{nullptr};
    OesFilter* mOesFilter{nullptr};
    GLfloat *mCamMatrix{nullptr};
};

#endif //ENGINE_CAMERARENDER_H
