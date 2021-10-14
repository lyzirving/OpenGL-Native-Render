//
// Created by liuyuzhou on 2021/10/13.
//
#include "ImageFaceDetector.h"
#include "Point.h"
#include "opencv2/opencv.hpp"
#include "dlib/opencv/cv_image.h"

#define TAG "ImageFaceDetector"
#define CLASSIFIER_PATH "/storage/emulated/0/Android/data/com.render.demo/files/Documents/haarcascade_frontalface_default.xml"

void ImageFaceDetector::buildTracker() {
    releaseTracker();
    mFaceClassifier = new cv::CascadeClassifier;
    bool res = mFaceClassifier->load(CLASSIFIER_PATH);
    LogUtil::logI(TAG, {"buildTracker: res = ", std::to_string(res)});
}

void ImageFaceDetector::handleOtherMessage(JNIEnv *env, const EventMessage &msg) {
    switch (msg.what) {
        case EventType::EVENT_FACE_LAND_MARK_TRACK: {
            LogUtil::logI(TAG, {"handleOtherMessage: land mark track"});
            handleFaceLandMarkTrack(env);
            break;
        }
        default: {
            LogUtil::logI(TAG, {"handleOtherMessage: default"});
            break;
        }
    }
}

void ImageFaceDetector::handleFaceLandMarkTrack(JNIEnv *env) {
    std::shared_ptr<Image> img = mImgQueue->dequeueNotWait();
    if (img != nullptr && img->data != nullptr) {
        trackFaceLandMark(env, img->data, img->width, img->height, img->channel);
    } else {
        LogUtil::logI(TAG, {"handleFaceLandMarkTrack: track face invalid data"});
    }
}

void ImageFaceDetector::releaseTracker() {
    delete mFaceClassifier;
    mFaceClassifier = nullptr;
}

void ImageFaceDetector::setFaceListener(void *listener) {
    mFaceListener = listener;
}

void ImageFaceDetector::setLandMarkDetectCallback(
        void *(*callbackLandMarkDetect)(void *, void *, int, ...)) {
    mCallbackLandMarkDetect = callbackLandMarkDetect;
}

void ImageFaceDetector::setTrackPauseCallback(void *(*callback)(void* env, void* arg)) {
    mCallbackPauseTrack = callback;
}

void ImageFaceDetector::trackFaceLandMark(JNIEnv *env, unsigned char *data, int width, int height,
                                          int channel) {
    cv::Mat src(height, width, CV_8UC4, data);
    cv::Mat gray;
    cv::cvtColor(src, gray, cv::COLOR_RGBA2GRAY);
    std::vector<cv::Rect> faces;
    mFaceClassifier->detectMultiScale(gray, faces, 1.1f, 4,
                                      cv::CASCADE_FIND_BIGGEST_OBJECT | cv::CASCADE_DO_ROUGH_SEARCH,
                                      cv::Size(30, 30));
    if (faces.empty()) {
        LogUtil::logI(TAG, {"trackFaceLandMark: no face detected"});
    } else {
        LogUtil::logI(TAG, {"trackFaceLandMark"});
        cv::Mat bgrMat;
        cv::cvtColor(src, bgrMat, cv::COLOR_RGBA2BGR);
        dlib::cv_image<dlib::bgr_pixel> bgrPixel(bgrMat);
        //only support lift one face now
        dlib::rectangle dFaceRect;
        dFaceRect.set_left(faces[0].x);
        dFaceRect.set_top(faces[0].y);
        dFaceRect.set_right(faces[0].x + faces[0].width);
        dFaceRect.set_bottom(faces[0].y + faces[0].height);
        dlib::full_object_detection detection = mShapePredictor(bgrPixel, dFaceRect);
        Point lhsDst;
        Point lhsCtrl;
        Point rhsDst;
        Point rhsCtrl;
        float divider = 15;
        lhsDst.x = (float) (detection.part(0).x() + detection.part(8).x()) / 2;
        lhsDst.y = (float) (detection.part(0).y() + detection.part(8).y()) / 2;

        lhsCtrl.x = detection.part(0).x() -
                    (float) (detection.part(27).x() - detection.part(0).x()) / divider;
        lhsCtrl.y = lhsDst.y + (lhsDst.x - lhsCtrl.x) *
                               (float) (detection.part(8).x() - detection.part(0).x()) /
                               (float) (detection.part(8).y() - detection.part(0).y());

        rhsDst.x = (float) (detection.part(8).x() + detection.part(16).x()) / 2;
        rhsDst.y = (float) (detection.part(8).y() + detection.part(16).y()) / 2;

        rhsCtrl.x = detection.part(16).x() +
                    (float) (detection.part(16).x() - detection.part(27).x()) / divider;
        rhsCtrl.y = rhsDst.y + (rhsDst.x - rhsCtrl.x) *
                               (float) (detection.part(16).x() - detection.part(8).x()) /
                               (float) (detection.part(16).y() - detection.part(8).y());
        if (mCallbackLandMarkDetect != nullptr) {
            mCallbackLandMarkDetect(env, mFaceListener, 4, &lhsDst, &lhsCtrl, &rhsDst, &rhsCtrl);
        }
    }
}

