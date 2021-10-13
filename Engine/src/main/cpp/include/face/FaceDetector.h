//
// Created by liuyuzhou on 2021/9/28.
//
#ifndef ENGINE_FACEDETECTOR_H
#define ENGINE_FACEDETECTOR_H

#include <jni.h>

#include "dlib/image_processing/shape_predictor.h"
#include "dlib/opencv/cv_image.h"
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
    void notifyLandMarkDetect(JNIEnv* env, jclass listenerClass, const std::vector<cv::Rect>& faces, const dlib::cv_image<dlib::bgr_pixel>& bgrPixel);
    void pause();
    void prepare(JNIEnv* env);
    void quitAndWait();
    void releaseTracker();
    void start();
    void setCallback(void*(*pCallbackStart)(void *argStart),
                     void*(*pCallbackStop)(void *argStop),
                     void*(*pCallbackFaceDetect)(void* env, void* arg0, int argNum, ...),
                     void* callback);

private:
    void trackFace(JNIEnv* env, unsigned char* data, int width, int height, int channel);
    void trackFaceNative(JNIEnv* env, unsigned char* data, int width, int height, int channel);
    void writePngImage(const unsigned char* data, int width, int height, int channel);

    ObjectQueue<EventMessage>* mMessageQueue{nullptr};
    PointerQueue<Image>* mImgQueue{nullptr};
    volatile render::Status mStatus{render::Status::STATUS_IDLE};
    pthread_mutex_t mQuitMutexLock{};
    pthread_cond_t mQuitCondLock{};

    pthread_mutex_t mStatusLock{};

    cv::Ptr<cv::DetectionBasedTracker> mFaceTracker{nullptr};
    dlib::shape_predictor mShapePredictor{};
    ValidPtr<_jobject>* mJavaListener{nullptr};

    void*(*mTrackStartCallback)(void *args){nullptr};
    void*(*mTrackStopCallback)(void *args){nullptr};
    void*(*mFaceDetectCallback)(void* env, void* arg0, int argNum, ...){nullptr};
    void* mCallback{nullptr};
};

#endif //ENGINE_FACEDETECTOR_H
