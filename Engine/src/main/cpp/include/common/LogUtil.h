//
// Created by liuyuzhou on 2021/9/7.
//

#ifndef ENGINE_LOGUTIL_H
#define ENGINE_LOGUTIL_H

#include <string>

class LogUtil {
public:
    static void logD(std::string tag, std::initializer_list<std::string> msg);
    static void logI(std::string tag, std::initializer_list<std::string> msg);
    static void logE(std::string tag, std::initializer_list<std::string> msg);
};

#endif //ENGINE_LOGUTIL_H
