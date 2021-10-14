//
// Created by liuyuzhou on 2021/10/13.
//
#ifndef ENGINE_BASEFACEDETECTOR_H
#define ENGINE_BASEFACEDETECTOR_H

#include "Common.h"
#include "dlib/image_processing/shape_predictor.h"
#include "CustomQueue.h"
#include "EventMessage.h"
#include "Image.h"

class BaseFaceDetector {
public:
    BaseFaceDetector();
    ~BaseFaceDetector();

    void changeStatus(render::Status newState);
    void enqueueImg(unsigned char* data, int width, int height, int channel, EventType type);
    void execute(bool start);
    render::Status getStatus();
    bool isRunning();
    void loop(JNIEnv* env);
    void prepare(JNIEnv* env);
    void quitAndWait();

    virtual void setLandMarkDetectCallback(
            void *(*callbackLandMarkDetect)(void *env, void *arg0, int argNum, ...)) = 0;
    virtual void setTrackPauseCallback(void*(*callback)(void* env, void* arg)) = 0;
    virtual void setFaceListener(void *listener) = 0;

protected:
    void buildEnv();
    void copyImageLocal(const unsigned char *data, int width, int height, int channel);

    virtual void buildFacePredictor();
    virtual void buildTracker() = 0;
    virtual void handleFinishBuildEnv();
    virtual void handleOtherMessage(JNIEnv* env, const EventMessage& msg) = 0;
    virtual void notifyRunTrack(JNIEnv* env);
    virtual void notifyPauseTrack(JNIEnv* env);
    virtual void notifyQuitTrack(JNIEnv* env);
    virtual void releaseTracker() = 0;
    virtual void trackFaceLandMark(JNIEnv* env, unsigned char* data, int width, int height, int channel) = 0;

    ObjectQueue<EventMessage>* mMessageQueue{nullptr};
    PointerQueue<Image>* mImgQueue{nullptr};
    dlib::shape_predictor mShapePredictor{};

    void*(*mCallbackLandMarkDetect)(void* env, void* arg0, int argNum, ...){nullptr};
    void*(*mCallbackPauseTrack)(void* env, void* arg0);
    void* mFaceListener{nullptr};

private:

    volatile render::Status mStatus{render::Status::STATUS_IDLE};
    pthread_mutex_t mStatusLock{};

    pthread_mutex_t mQuitMutexLock{};
    pthread_cond_t mQuitCondLock{};
};

#endif //ENGINE_BASEFACEDETECTOR_H
