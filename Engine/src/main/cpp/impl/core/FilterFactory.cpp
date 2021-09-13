//
// Created by liuyuzhou on 2021/9/9.
//
#include "LogUtil.h"
#include "FilterFactory.h"
#include "ContrastFilter.h"
#include "SharpenFilter.h"
#include "SaturationFilter.h"

#define TAG "FilterFactory"

std::shared_ptr<BaseFilter> FilterFactory::makeFilter(const std::string &filterType) {
    if (filterType == "CONTRAST") {
        LogUtil::logI(TAG, {"makeFilter: contrast"});
        return std::make_shared<ContrastFilter>();
    } else if (filterType == "SHARPEN") {
        return std::make_shared<SharpenFilter>();
    } else if (filterType == "SATURATION") {
        return std::make_shared<SaturationFilter>();
    } else {
        LogUtil::logI(TAG, {"makeFilter: none"});
        return nullptr;
    }
}

