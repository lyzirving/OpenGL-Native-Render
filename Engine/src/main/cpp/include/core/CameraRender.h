//
// Created by liuyuzhou on 2021/9/17.
//
#ifndef ENGINE_CAMERARENDER_H
#define ENGINE_CAMERARENDER_H

#include "GlUtil.h"
#include "RenderEglBase.h"
#include "EventQueue.h"
#include "WorkQueue.h"
#include "ScreenFilter.h"
#include "OesFilter.h"

class CameraMetaData {
public:
    static const int LENS_FACING_FRONT = 0;
    static const int LENS_FACING_BACK = 1;
    static const int LENS_FACING_EXTERNAL = 2;

    int previewWidth{0};
    int previewHeight{0};
    int frontType{LENS_FACING_BACK};
private:
};

class CameraRender {
public:

    CameraRender();
    static bool registerSelf(JNIEnv *env);
    void buildOesTexture();
    void buildCameraTransMatrix();
    void drawFrame();
    void enqueueMessage(EventType what);
    bool initialized();
    void notifyEnvPrepare(JNIEnv* env, jobject listener);
    void notifyEnvRelease(JNIEnv* env, jobject listener);
    void render(JNIEnv* env);
    void release(JNIEnv* env);
    void releaseBeforeEnvDestroy(JNIEnv* env);
    void runBeforeDraw();
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
    GLuint mOesTexture{0};
    RenderStatus mStatus = RenderStatus::STATUS_IDLE;
    jobject mSurfaceTexture{nullptr};

    CameraMetaData* mMetadata{nullptr};
    ScreenFilter* mScreenFilter{nullptr};
    OesFilter* mOesFilter{nullptr};
    GLfloat *mCamMatrix{nullptr};
};

#endif //ENGINE_CAMERARENDER_H
