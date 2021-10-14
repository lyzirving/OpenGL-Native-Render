//
// Created by liuyuzhou on 2021/10/13.
//
#ifndef RENDERDEMO_CAMFACEDETECTOR_H
#define RENDERDEMO_CAMFACEDETECTOR_H

#include "BaseFaceDetector.h"
#include "CascadeDetectorAdapter.h"
#include "dlib/opencv/cv_image.h"

class CamFaceDetector : public BaseFaceDetector {
public:

    void setFaceListener(void *listener) override;
    void setLandMarkDetectCallback(void *(*callbackLandMarkDetect)(void *, void *, int, ...)) override;
    void setTrackPauseCallback(void *(*callback)(void* env, void* arg0)) override;

protected:

    void buildTracker() override;
    void handleOtherMessage(JNIEnv *env, const EventMessage &msg) override;
    void releaseTracker() override;
    void trackFaceLandMark(JNIEnv *env, unsigned char *data, int width, int height,
                           int channel) override;

private:
    void handleFaceLandMarkTrack(JNIEnv* env);

    cv::Ptr<cv::DetectionBasedTracker> mFaceTracker{nullptr};
};

#endif //RENDERDEMO_CAMFACEDETECTOR_H
