//
// Created by lyzirving on 2021/9/25.
//
#ifndef ENGINE_IMAGERENDER_H
#define ENGINE_IMAGERENDER_H

#include "BaseRender.h"
#include "ImageFaceDetector.h"
#include "BackgroundFilter.h"
#include "MaskFilter.h"
#include "FaceLiftFilter.h"
#include "PlaceHolderFilter.h"
#include "Point.h"

class ImageRender : public BaseRender {
public:
    static bool registerSelf(JNIEnv *env);

    void adjustProperty(const char *filterType, const char *property, int progress) override;
    void drawFrame() override;

    void handleFaceLiftLandMarkTrack(Point* lhsDst, Point* lhsCtrl, Point* rhsDst, Point* rhsCtrl);
    void notifyImageLandMarkTrackStart(JNIEnv* env);
    void notifyImageLandMarkTrackFinish(JNIEnv* env);
    void setResource(JNIEnv* env, jobject bitmap);
    void trackFace(bool trackFace);

protected:

    void handleEnvPrepare(JNIEnv *env) override;
    void handleOtherMessage(JNIEnv *env, const EventMessage& msg) override;
    void handlePreDraw(JNIEnv *env) override;
    void handlePostDraw(JNIEnv *env) override;
    void handleRenderEnvPause(JNIEnv *env) override;
    void handleRenderEnvResume(JNIEnv *env) override;
    void handleRenderEnvDestroy(JNIEnv *env) override;
    void handleSurfaceChange(JNIEnv *env) override;

    void handleDownloadRawPreview();

private:
    void drawFaceLift(GLuint* inputTexture, int drawCount);

    MaskFilter* mMaskFilter{nullptr};
    BackgroundFilter* mBackgroundFilter{nullptr};
    FaceLiftFilter* mFaceLiftFilter{nullptr};
    PlaceHolderFilter* mPlaceHolderFilter{nullptr};
    ImageFaceDetector* mImageFaceDetector{nullptr};

    GLuint mDownloadBuffer{0};
    render::DownloadMode mDownloadMode{render::DownloadMode::MODE_FACE_DETECT};
};

#endif //ENGINE_IMAGERENDER_H
