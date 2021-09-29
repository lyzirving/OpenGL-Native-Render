//
// Created by liuyuzhou on 2021/9/28.
//
#ifndef ENGINE_FACEDETECTOR_H
#define ENGINE_FACEDETECTOR_H

#include <jni.h>

#include "CascadeDetectorAdapter.h"
#include "CustomQueue.h"
#include "EventMessage.h"
#include "Image.h"
#include "Common.h"

class FaceDetector {
public:

    FaceDetector();
    ~FaceDetector();

    void buildTracker();
    void enqueueImg(unsigned char* data, int width, int height, int channel, EventType type);
    bool isRunning();
    void loop(JNIEnv* env);
    void prepare(JNIEnv* env);
    void quit();
    void quitAndWait();
    void releaseTracker();

private:
    void trackFace(unsigned char* data, int width, int height, int channel);
    void writePngImage(const unsigned char* data, int width, int height, int channel);

    ObjectQueue<EventMessage>* mMessageQueue{nullptr};
    PointerQueue<Image>* mImgQueue{nullptr};
    volatile render::Status mStatus{render::Status::STATUS_IDLE};
    pthread_mutex_t mQuitMutexLock{};
    pthread_cond_t mQuitCondLock{};

    cv::Ptr<cv::DetectionBasedTracker> mFaceTracker{nullptr};
};

#endif //ENGINE_FACEDETECTOR_H
