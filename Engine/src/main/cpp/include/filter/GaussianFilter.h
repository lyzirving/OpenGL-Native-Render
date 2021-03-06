//
// Created by liuyuzhou on 2021/9/14.
//
#ifndef ENGINE_GAUSSIANFILTER_H
#define ENGINE_GAUSSIANFILTER_H

#include "GaussianOnePassFilter.h"

class GaussianFilter : public BaseFilter {
public:
    GaussianFilter();
    ~GaussianFilter();

    void adjustProperty(const char *property, int progress) override;
    void destroy() override;
    void init() override;
    void initFrameBuffer() override;
    void initTexture() override;
    GLint onDraw(GLint inputFrameBufferId, GLint inputTextureId) override;
    void onPause() override;
    void setOutputSize(GLint width, GLint height) override;
    void setBlurSize(float horBlur, float verBlur);

private:
    GaussianOnePassFilter* mHorFilter = nullptr;
    GaussianOnePassFilter* mVerFilter = nullptr;
    GLuint mFrameBufferId{0};
    float mHorBlurSize{0};
    float mVerBlurSize{0};
};

#endif //RENDERDEMO_GAUSSIANFILTER_H
