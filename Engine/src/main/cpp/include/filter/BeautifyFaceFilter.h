//
// Created by liuyuzhou on 2021/10/15.
//
#ifndef ENGINE_BEAUTIFYFACEFILTER_H
#define ENGINE_BEAUTIFYFACEFILTER_H

#include "BaseFilter.h"
#include "SkinFilter.h"
#include "GaussianFilter.h"
#include "HighPassFilter.h"
#include "SkinBuffFilter.h"

class BeautifyFaceFilter : public BaseFilter {
public:
    BeautifyFaceFilter();
    ~BeautifyFaceFilter();

    void adjustProperty(const char *property, int progress) override;
    void destroy() override;
    void init() override;
    void initFrameBuffer() override;
    void initTexture() override;
    GLint onDraw(GLint inputTextureId) override;
    void onPause() override;
    void onResume() override;
    void setOutputSize(GLint width, GLint height) override;

private:
    SkinFilter* mSkinFilter{nullptr};
    GaussianFilter* mGaussianFilter{nullptr};
    HighPassFilter* mHighPassFilter{nullptr};
    SkinBuffFilter* mSkinBuffFilter{nullptr};

    GLuint* mFrameBuffers{nullptr};
    GLuint* mTextures{nullptr};
};

#endif //ENGINE_BEAUTIFYFACEFILTER_H
