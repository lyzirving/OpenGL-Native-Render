//
// Created by liuyuzhou on 2021/9/29.
//
#include "CascadeDetectorAdapter.h"

CascadeDetectorAdapter::~CascadeDetectorAdapter() = default;

void CascadeDetectorAdapter::detect(const cv::Mat &image, std::vector<cv::Rect> &objects) {
    mDetector->detectMultiScale(image, objects, scaleFactor, minNeighbours, 0, minObjSize, maxObjSize);
}
