//
// Created by liuyuzhou on 2021/9/8.
//
#ifndef ENGINE_SCREENFILTER_H
#define ENGINE_SCREENFILTER_H

#include "BaseFilter.h"

class ScreenFilter : public BaseFilter {
public:
    ScreenFilter();
    ~ScreenFilter();

    void flip(bool horizontal, bool vertical);
    void init() override;
    void initHandler() override;
    void loadShader() override;
    GLint onDraw(GLint inputTextureId) override;
private:
    int mTransHandler{0};
};

#endif //ENGINE_SCREENFILTER_H
