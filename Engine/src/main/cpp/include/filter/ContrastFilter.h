//
// Created by liuyuzhou on 2021/9/9.
//
#ifndef ENGINE_CONTRASTFILTER_H
#define ENGINE_CONTRASTFILTER_H

#include "BaseFilter.h"

/**
 * adjust contrast, mContrast ranges from [0, 2], the maximum is 4;
 * when mContrast is 1, the state is default;
 */
class ContrastFilter : public BaseFilter {
public:

    void adjust(int progress) override;
    void init() override;
    void loadShader() override;
    void initHandler() override;
    GLint onDraw(GLint inputTextureId) override;

private:
    GLint mContrastHandler{0};
    GLfloat mContrast{1};
};

#endif //ENGINE_CONTRASTFILTER_H
