//
// Created by liuyuzhou on 2021/9/28.
//
#ifndef ENGINE_FACEDETECTOR_H
#define ENGINE_FACEDETECTOR_H

#include <jni.h>

#include "dlib/image_processing/shape_predictor.h"
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
    void changeStatus(render::Status newState);
    void enqueueImg(unsigned char* data, int width, int height, int channel, EventType type);
    bool isRunning();
    render::Status getStatus();
    void loop(JNIEnv* env);
    void notifyStartTrack(JNIEnv* env);
    void notifyStopTrackFace(JNIEnv* env);
    void pause();
    void prepare(JNIEnv* env);
    void quitAndWait();
    void releaseTracker();
    void start();

private:
    void trackFace(JNIEnv* env, unsigned char* data, int width, int height, int channel);
    void writePngImage(const unsigned char* data, int width, int height, int channel);

    ObjectQueue<EventMessage>* mMessageQueue{nullptr};
    PointerQueue<Image>* mImgQueue{nullptr};
    volatile render::Status mStatus{render::Status::STATUS_IDLE};
    pthread_mutex_t mQuitMutexLock{};
    pthread_cond_t mQuitCondLock{};

    pthread_mutex_t mStatusLock{};

    cv::Ptr<cv::DetectionBasedTracker> mFaceTracker{nullptr};
    dlib::shape_predictor mShapePredictor{};
    ValidPtr<_jobject>* mListener{nullptr};
};

#endif //ENGINE_FACEDETECTOR_H
