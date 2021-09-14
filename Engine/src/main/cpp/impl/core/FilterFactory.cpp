//
// Created by liuyuzhou on 2021/9/9.
//
#include "LogUtil.h"
#include "FilterFactory.h"

#include "ContrastFilter.h"
#include "SharpenFilter.h"
#include "SaturationFilter.h"
#include "ExposureFilter.h"
#include "HighlightShadowFilter.h"

#define TAG "FilterFactory"

std::shared_ptr<BaseFilter> FilterFactory::makeFilter(const std::string &filterType) {
    if (filterType == EngineUtil::FILTER_CONTRAST) {
        LogUtil::logI(TAG, {"makeFilter: contrast"});
        return std::make_shared<ContrastFilter>();
    } else if (filterType == EngineUtil::FILTER_SHARPEN) {
        return std::make_shared<SharpenFilter>();
    } else if (filterType == EngineUtil::FILTER_SATURATION) {
        return std::make_shared<SaturationFilter>();
    } else if (filterType == EngineUtil::FILTER_EXPOSURE) {
        return std::make_shared<ExposureFilter>();
    } else if (filterType == EngineUtil::FILTER_HIGHLIGHT_SHADOW) {
        return std::make_shared<HighlightShadowFilter>();
    } else {
        LogUtil::logI(TAG, {"makeFilter: none"});
        return nullptr;
    }
}

