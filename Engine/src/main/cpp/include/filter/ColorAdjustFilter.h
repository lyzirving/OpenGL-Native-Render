//
// Created by liuyuzhou on 2021/10/14.
//
#ifndef ENGINE_COLORADJUSTFILTER_H
#define ENGINE_COLORADJUSTFILTER_H

#include "BaseFilter.h"

/**
 * sharpen the picture
 * mSharpness: from -4.0 to 4.0, with 0.0 as the normal level
 *
 * adjust contrast, mContrast ranges from [0, 2], the maximum is 4;
 * when mContrast is 1, the state is default;
 *
 * degree of saturation or desaturation to apply to the image
 * 0.0 - 2.0, with 1.0 as the default
 */
class ColorAdjustFilter : public BaseFilter {
public:
    void adjustProperty(const char *property, int progress) override;
    void initHandler() override;
    void loadShader() override;
    GLint onDraw(GLint inputTextureId) override;

private:
    GLint mImgWidthHandler{0};
    GLint mImgHeightHandler{0};
    GLint mSharpnessHandler{0};
    GLint mContrastHandler{0};
    GLint mSaturationHandler{0};

    GLfloat mSharpness{0};
    GLfloat mContrast{1};
    GLfloat mSaturation{1};
};

#endif //ENGINE_COLORADJUSTFILTER_H
