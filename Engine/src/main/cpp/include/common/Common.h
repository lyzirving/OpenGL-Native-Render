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

    static const char* FILTER_TYPE_GROUP = "GROUP";
    static const char* FILTER_CONTRAST = "CONTRAST";
    static const char* FILTER_SHARPEN = "SHARPEN";
    static const char* FILTER_SATURATION = "SATURATION";
    static const char* FILTER_EXPOSURE = "EXPOSURE";
    static const char* FILTER_HIGHLIGHT_SHADOW = "HIGHLIGHT_SHADOW";
    static const char* FILTER_GAUSSIAN = "GAUSSIAN";

    static const char* FILTER_PROP_HIGHLIGHT = "HIGHLIGHT";
    static const char* FILTER_PROP_SHADOW = "SHADOW";
    static const char* FILTER_PROP_HOR_GAUSSIAN = "HOR_GAUSSIAN";
    static const char* FILTER_PROP_VER_GAUSSIAN = "VER_GAUSSIAN";

    static void createClassMap() {
        if (gClassMap == nullptr) { gClassMap = new std::map<std::string, jobject>; }
    }

    static void getJvm(JNIEnv* env) {
        if (gJvm == nullptr) { env->GetJavaVM(&gJvm); }
    }

    static long long getCurrentTimeMs() {
        timeval tv{};
        gettimeofday(&tv, nullptr);
        return ((long long)tv.tv_sec) * 1000 + tv.tv_usec / 1000;
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
