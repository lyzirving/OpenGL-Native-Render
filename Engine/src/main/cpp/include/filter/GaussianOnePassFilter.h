//
// Created by liuyuzhou on 2021/9/14.
//
#ifndef ENGINE_GAUSSIANONEPASSFILTER_H
#define ENGINE_GAUSSIANONEPASSFILTER_H

#include "BaseFilter.h"

enum class Orientation : uint8_t {
    HORIZONTAL = 0,
    VERTICAL = 1
};

class GaussianOnePassFilter : public BaseFilter {
public:

    GaussianOnePassFilter(Orientation orientation);
    ~GaussianOnePassFilter();
    void adjustHorBlur(float blurSize);
    void adjustVerBlur(float blurSize);
    void init() override;
    void flip(bool horFlip, bool verFlip);
    void initHandler() override;
    void loadShader() override;
    GLint onDraw(GLint inputTextureId) override;
    void setBlurSize(const char* prop, float blur);

private:
    Orientation mOrientation{Orientation::HORIZONTAL};
    int mTextureWidthOffsetHandler{0};
    int mTextureHeightOffsetHandler{0};
    int mMatrixHandler{0};
    float mHorBlurSize{0};
    float mVerBlurSize{0};
};

#endif //ENGINE_GAUSSIANONEPASSFILTER_H
