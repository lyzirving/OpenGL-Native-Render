//
// Created by liuyuzhou on 2021/9/29.
//
#ifndef ENGINE_CASCADEDETECTORADAPTER_H
#define ENGINE_CASCADEDETECTORADAPTER_H

#include "opencv2/opencv.hpp"

class CascadeDetectorAdapter : public cv::DetectionBasedTracker::IDetector {
public:
    CascadeDetectorAdapter(cv::Ptr<cv::CascadeClassifier> detector) : cv::DetectionBasedTracker::IDetector(), mDetector(detector) {
        CV_Assert(detector);
    }
    virtual ~CascadeDetectorAdapter();

    void detect(const cv::Mat &image, std::vector<cv::Rect> &objects) override;

private:
    cv::Ptr<cv::CascadeClassifier> mDetector;
};

#endif //ENGINE_CASCADEDETECTORADAPTER_H
