//
// Created by liuyuzhou on 2021/9/24.
//
#ifndef ENGINE_CAMRENDER_H
#define ENGINE_CAMRENDER_H

#include "BaseRender.h"
#include "PlaceHolderFilter.h"
#include "OesFilter.h"
#include "FaceLiftFilter.h"
#include "DownloadPixelFilter.h"
#include "CamFaceDetector.h"
#include "Point.h"

class CamRender : public BaseRender {
public:
    static bool registerSelf(JNIEnv *env);

    void adjustProperty(const char *filterType, const char *property, int progress) override;
    void drawFrame() override;
    GLuint getContentTexture() override;
    void trackFace(bool start) override;

    void handleLandMarkTrack(Point* lhsDst, Point* lhsCtrl, Point* rhsDst, Point* rhsCtrl);
    void setCameraMetadata(JNIEnv* env, jobject data);
    void setPreview(GLint width, GLint height);
    void setSurfaceTexture(JNIEnv* env, jobject surfaceTexture);

protected:

    void handleDownloadPixel(GLuint* inputTexture, int drawCount);
    void handleEnvPrepare(JNIEnv *env) override;
    void handleFaceTrackStart(JNIEnv *env);
    void handleFaceTrackStop(JNIEnv *env);
    void handleOtherMessage(JNIEnv* env, const EventMessage& msg) override;
    void handlePreDraw(JNIEnv *env) override;
    void handlePostDraw(JNIEnv *env) override;
    void handleRenderEnvPause(JNIEnv *env) override;
    void handleRenderEnvResume(JNIEnv *env) override;
    void handleRenderEnvDestroy(JNIEnv *env) override;
    void handleSurfaceChange(JNIEnv *env) override;
    void buildOesTexture();
    void downloadPreview(GLuint frameBuffer);
    void notifyEnvOesTextureCreate(JNIEnv* env, jobject listener, int oesTexture);
    void updateTexImg(JNIEnv* env);

private:
    GLint mPreviewWidth{0};
    GLint mPreviewHeight{0};
    GLuint mOesTexture{0};
    GLuint* mDownloadBuffer{nullptr};

    jobject mSurfaceTexture{nullptr};
    render::CameraMetaData* mCamMetaData{nullptr};

    PlaceHolderFilter* mPlaceHolderFilter{nullptr};
    OesFilter* mOesFilter{nullptr};
    DownloadPixelFilter* mDownloadFilter{nullptr};
    FaceLiftFilter* mFaceLiftFilter{nullptr};

    CamFaceDetector* mCamFaceDetector{nullptr};

    int mDownloadFreeIndex{0};
    render::DownloadMode mDownloadMode{render::DownloadMode::MODE_FACE_DETECT};
};

#endif //ENGINE_CAMRENDER_H
