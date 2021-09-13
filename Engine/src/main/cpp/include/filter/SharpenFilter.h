//
// Created by liuyuzhou on 2021/9/13.
//
#ifndef ENGINE_SHARPENFILTER_H
#define ENGINE_SHARPENFILTER_H

#include "BaseFilter.h"

/**
 * sharpen the picture
 * mSharpness: from -4.0 to 4.0, with 0.0 as the normal level
 */
class SharpenFilter : public BaseFilter {
public:

    void adjust(int progress) override;
    void init() override;
    void initHandler() override;
    void loadShader() override;
    GLint onDraw(GLint inputTextureId) override;
    void setOutputSize(GLint width, GLint height) override;
private:
    int mImgWidthHandler{0};
    int mImgHeightHandler{0};
    int mSharpnessHandler{0};
    float mSharpness{0};
    float mWidthFactor{0};
    float mHeightFactor{0};
};

#endif //ENGINE_SHARPENFILTER_H
