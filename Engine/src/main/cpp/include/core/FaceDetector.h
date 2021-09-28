//
// Created by liuyuzhou on 2021/9/28.
//
#ifndef ENGINE_FACEDETECTOR_H
#define ENGINE_FACEDETECTOR_H

#include <jni.h>
#include "CustomQueue.h"
#include "EventMessage.h"
#include "Image.h"
#include "Common.h"

class FaceDetector {
public:

    FaceDetector();
    ~FaceDetector();

    void copyAndEnqueueData(const unsigned char* data, int width, int height, int channel);
    void loop(JNIEnv* env);
    void prepare(JNIEnv* env);
    void quit();

private:

    void writeImageToFile(const unsigned char* data, int size);

    ObjectQueue<EventMessage>* mMessageQueue{nullptr};
    ObjectQueue<Image>* mImageQueue{nullptr};
    render::Status mStatus{render::Status::STATUS_IDLE};
};

#endif //ENGINE_FACEDETECTOR_H
