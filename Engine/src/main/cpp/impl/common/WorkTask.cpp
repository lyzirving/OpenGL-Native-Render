//
// Created by liuyuzhou on 2021/9/6.
//
#include "WorkTask.h"
#include "BaseFilter.h"

#define TAG "WorkTask"

void WorkTask::run() {}

void FilterInitTask::run() {
    auto* filter = static_cast<BaseFilter *>(mObj);
    filter->init();
}
