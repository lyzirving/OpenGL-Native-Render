//
// Created by lyzirving on 2021/11/21.
//
#ifndef ENGINE_DEFAULTFILTER_H
#define ENGINE_DEFAULTFILTER_H

#include "BaseFilter.h"

class DefaultFilter : public BaseFilter {
public:
    GLint onDraw(GLint inputTextureId) override;
};

#endif //ENGINE_DEFAULTFILTER_H
