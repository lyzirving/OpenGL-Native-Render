//
// Created by liuyuzhou on 2021/9/9.
//
#ifndef ENGINE_FILTERFACTORY_H
#define ENGINE_FILTERFACTORY_H

#include "BaseFilter.h"
#include "Static.h"
#include "string"

class FilterFactory {
public:
    static std::shared_ptr<BaseFilter> makeFilter(const std::string& filterType);
private:
};

#endif //ENGINE_FILTERFACTORY_H
