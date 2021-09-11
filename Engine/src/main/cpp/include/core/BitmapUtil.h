//
// Created by liuyuzhou on 2021/9/7.
//
#ifndef ENGINE_BITMAPUTIL_H
#define ENGINE_BITMAPUTIL_H

#include <android/bitmap.h>

class BitmapUtil {
public:
    static BitmapUtil* self();
private:
    BitmapUtil() {}
    ~BitmapUtil() {}
};

#endif //ENGINE_BITMAPUTIL_H
