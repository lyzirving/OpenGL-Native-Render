//
// Created by liuyuzhou on 2021/9/9.
//
#include "LogUtil.h"
#include "FilterFactory.h"
#include "Common.h"

#include "ExposureFilter.h"
#include "HighlightShadowFilter.h"
#include "GaussianFilter.h"
#include "ColorAdjustFilter.h"

#define TAG "FilterFactory"

std::shared_ptr<BaseFilter> FilterFactory::makeFilter(const std::string &filterType) {
    if (filterType == render::FILTER_EXPOSURE) {
        return std::make_shared<ExposureFilter>();
    } else if (filterType == render::FILTER_HIGHLIGHT_SHADOW) {
        return std::make_shared<HighlightShadowFilter>();
    } else if (filterType == render::FILTER_GAUSSIAN) {
        return std::make_shared<GaussianFilter>();
    } else if (filterType == render::FILTER_COLOR_ADJUST) {
        return std::make_shared<ColorAdjustFilter>();
    } else {
        LogUtil::logI(TAG, {"makeFilter: none filter found for ", filterType});
        return nullptr;
    }
}

