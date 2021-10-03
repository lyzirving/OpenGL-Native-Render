//
// Created by liuyuzhou on 2021/9/28.
//
#ifndef ENGINE_FACEDETECTOR_H
#define ENGINE_FACEDETECTOR_H

#include <jni.h>

#include "ValidPtr.h"
#include "CascadeDetectorAdapter.h"
#include "CustomQueue.h"
#include "EventMessage.h"
#include "Image.h"
#include "Common.h"

class FaceDetector {
public:

    FaceDetector();
    FaceDetector(ValidPtr<_jobject>* listener);
    ~FaceDetector();

    static bool registerSelf(JNIEnv *env);

    void buildTracker();
    void enqueueImg(unsigned char* data, int width, int height, int channel, EventType type);
    bool isRunning();
    void loop(JNIEnv* env);
    void notifyFaceDetect(JNIEnv* env, const std::vector<cv::Rect>& faces);
    void notifyStartTrack(JNIEnv* env);
    void notifyStopTrackFace(JNIEnv* env);
    void prepare(JNIEnv* env);
    void quit();
    void quitAndWait();
    void releaseTracker();

private:
    void trackFace(JNIEnv* env, unsigned char* data, int width, int height, int channel);
    void writePngImage(const unsigned char* data, int width, int height, int channel);

    ObjectQueue<EventMessage>* mMessageQueue{nullptr};
    PointerQueue<Image>* mImgQueue{nullptr};
    volatile render::Status mStatus{render::Status::STATUS_IDLE};
    pthread_mutex_t mQuitMutexLock{};
    pthread_cond_t mQuitCondLock{};

    cv::Ptr<cv::DetectionBasedTracker> mFaceTracker{nullptr};
    ValidPtr<_jobject>* mListener{nullptr};
};

#endif //ENGINE_FACEDETECTOR_H
