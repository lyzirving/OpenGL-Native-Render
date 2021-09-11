//
// Created by liuyuzhou on 2021/9/9.
//
#include "LogUtil.h"
#include "FilterFactory.h"
#include "ContrastFilter.h"

#define TAG "FilterFactory"

std::shared_ptr<BaseFilter> FilterFactory::makeFilter(const std::string& filterType) {
    if (filterType == "CONTRAST") {
        LogUtil::logI(TAG, {"makeFilter: contrast"});
        return std::make_shared<ContrastFilter>();
    } else {
        LogUtil::logI(TAG, {"makeFilter: none"});
        return nullptr;
    }
}

