//
// Created by liuyuzhou on 2021/10/13.
//
#include "CamFaceDetector.h"
#include "Point.h"

#define TAG "CamFaceDetector"
#define CLASSIFIER_PATH "/storage/emulated/0/Android/data/com.render.demo/files/Documents/lbpcascade_frontalface_improved.xml"

void CamFaceDetector::buildTracker() {
    cv::Ptr<cv::CascadeClassifier> mainClassifier = cv::makePtr<cv::CascadeClassifier>(CLASSIFIER_PATH);
    cv::Ptr<CascadeDetectorAdapter> mainDetector = cv::makePtr<CascadeDetectorAdapter>(mainClassifier);

    cv::Ptr<cv::CascadeClassifier> subClassifier = cv::makePtr<cv::CascadeClassifier>(CLASSIFIER_PATH);
    cv::Ptr<CascadeDetectorAdapter> subDetector = cv::makePtr<CascadeDetectorAdapter>(subClassifier);

    cv::DetectionBasedTracker::Parameters params;
    mFaceTracker = cv::makePtr<cv::DetectionBasedTracker>(mainDetector, subDetector, params);
    bool res = mFaceTracker->run();
    LogUtil::logI(TAG, {"buildTracker: init face tracker, result = ", (res ? "true" : "false")});
}

void CamFaceDetector::handleOtherMessage(JNIEnv *env, const EventMessage &msg) {
    switch (msg.what) {
        case EventType::EVENT_FACE_LAND_MARK_TRACK: {
            handleFaceLandMarkTrack(env);
            break;
        }
        default: {
            LogUtil::logI(TAG, {"handleOtherMessage: default"});
            break;
        }
    }
}

void CamFaceDetector::handleFaceLandMarkTrack(JNIEnv* env) {
    std::shared_ptr<Image> img = mImgQueue->dequeueNotWait();
    if (img != nullptr && img->data != nullptr) {
        trackFaceLandMark(env, img->data, img->width, img->height, img->channel);
    } else {
        LogUtil::logI(TAG, {"handleFaceLandMarkTrack: track face invalid data"});
    }
}

void CamFaceDetector::releaseTracker() {
    if (mFaceTracker != nullptr) {
        mFaceTracker->stop();
        mFaceTracker.reset();
        mFaceTracker = nullptr;
    }
}

void CamFaceDetector::setFaceListener(void *listener) {
    mFaceListener = listener;
}

void CamFaceDetector::setLandMarkDetectCallback(
        void *(*callbackLandMarkDetect)(void *, void *, int, ...)) {
    mCallbackLandMarkDetect = callbackLandMarkDetect;
}

void CamFaceDetector::setTrackPauseCallback(void *(*callback)(void* env, void* arg0)) {}

void CamFaceDetector::trackFaceLandMark(JNIEnv *env, unsigned char *data, int width, int height,
                                        int channel) {
    cv::Mat src(height, width, CV_8UC4, data);
    cv::Mat gray, bgrMat;
    cv::cvtColor(src, gray, cv::COLOR_RGBA2GRAY);
    cv::cvtColor(src, bgrMat, cv::COLOR_RGBA2BGR);
    dlib::cv_image<dlib::bgr_pixel> bgrPixel(bgrMat);

    cv::equalizeHist(gray, gray);
    mFaceTracker->process(gray);
    std::vector<cv::Rect> faces;
    mFaceTracker->getObjects(faces);

    if (faces.empty()) {
        LogUtil::logI(TAG, {"trackFaceLandMark: no face detected"});
    } else {
        LogUtil::logI(TAG, {"trackFaceLandMark: face detected"});
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
