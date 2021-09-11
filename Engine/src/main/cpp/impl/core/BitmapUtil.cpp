//
// Created by liuyuzhou on 2021/9/3.
//
#include "LogUtil.h"
#include "Static.h"

BitmapUtil * BitmapUtil::self() {
    if (EngineUtil::gBitmapUtl == nullptr) {
        EngineUtil::lock();
        if (EngineUtil::gBitmapUtl == nullptr) {
            EngineUtil::gBitmapUtl = new BitmapUtil;
        }
        EngineUtil::unlock();
    }
    return EngineUtil::gBitmapUtl;
}
