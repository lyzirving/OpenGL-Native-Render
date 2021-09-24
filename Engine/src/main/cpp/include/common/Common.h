//
// Created by liuyuzhou on 2021/9/24.
//
#ifndef ENGINE_COMMON_H
#define ENGINE_COMMON_H

#include <map>
#include <jni.h>

namespace render {
    static JavaVM *gJvm = nullptr;
    static std::map<std::string, jobject>* gClassMap = nullptr;

    static void createClassMap() {
        if (gClassMap == nullptr) { gClassMap = new std::map<std::string, jobject>; }
    }

    class CameraMetaData {
    public:
        static const int LENS_FACING_FRONT = 0;
        static const int LENS_FACING_BACK = 1;
        static const int LENS_FACING_EXTERNAL = 2;

        int previewWidth{0};
        int previewHeight{0};
        int frontType{LENS_FACING_BACK};
    };

    enum class Status : uint8_t {
        STATUS_IDLE = 0,
        STATUS_PREPARE = 1,
        STATUS_RUN = 2,
        STATUS_PAUSE = 3,
        STATUS_DESTROY = 4
    };
}

#endif //ENGINE_COMMON_H
