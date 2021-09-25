//
// Created by liuyuzhou on 2021/9/9.
//
#include "LogUtil.h"
#include "FilterFactory.h"
#include "Common.h"

#include "ContrastFilter.h"
#include "SharpenFilter.h"
#include "SaturationFilter.h"
#include "ExposureFilter.h"
#include "HighlightShadowFilter.h"
#include "GaussianFilter.h"

#define TAG "FilterFactory"

std::shared_ptr<BaseFilter> FilterFactory::makeFilter(const std::string &filterType) {
    if (filterType == render::FILTER_CONTRAST) {
        return std::make_shared<ContrastFilter>();
    } else if (filterType == render::FILTER_SHARPEN) {
        return std::make_shared<SharpenFilter>();
    } else if (filterType == render::FILTER_SATURATION) {
        return std::make_shared<SaturationFilter>();
    } else if (filterType == render::FILTER_EXPOSURE) {
        return std::make_shared<ExposureFilter>();
    } else if (filterType == render::FILTER_HIGHLIGHT_SHADOW) {
        return std::make_shared<HighlightShadowFilter>();
    } else if (filterType == render::FILTER_GAUSSIAN) {
        return std::make_shared<GaussianFilter>();
    } else {
        LogUtil::logI(TAG, {"makeFilter: none filter found for ", filterType});
        return nullptr;
    }
}

