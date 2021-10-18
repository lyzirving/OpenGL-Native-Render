//
// Created by liuyuzhou on 2021/10/15.
//
#ifndef ENGINE_SKINBUFFFILTER_H
#define ENGINE_SKINBUFFFILTER_H

#include "BaseFilter.h"

class SkinBuffFilter : public BaseFilter {
public:

    void adjustProperty(const char *property, int progress) override;

    void initHandler() override;
    void loadShader() override;
    GLint onDraw(GLint inputTextureId) override;
    void setBlurTexture(GLuint blurTexture);
    void setHighPassBlurTexture(GLuint highPassBlurTexture);

private:
    GLint mBlurTextureSamplerHandler{0};
    GLint mHighPassBlurTextureSamplerHandler{0};
    GLint mSkinBuffIntensityHandler{0};

    GLint mImgWidthHandler{0};
    GLint mImgHeightHandler{0};
    GLint mSharpnessHandler{0};

    /**
     * we do not have to release this texture
     * it should be released by its constructor
     */
    GLuint mBlurTexture{0};
    /**
     * we do not have to release this texture
     * it should be released by its constructor
     */
    GLuint mHighPassBlurTexture{0};

    /**
     * mSkinBuffIntensity should only be 0~0.4
     */
    GLfloat mSkinBuffIntensity{0.2};
    GLfloat mSharpness{0.3};
};

#endif //ENGINE_SKINBUFFFILTER_H
