//
// Created by liuyuzhou on 2021/10/15.
//
#ifndef ENGINE_HIGHPASSFILTER_H
#define ENGINE_HIGHPASSFILTER_H

#include "BaseFilter.h"

class HighPassFilter : public BaseFilter {
public:

    void initHandler() override;
    void loadShader() override;
    GLint onDraw(GLint inputTextureId) override;
    void setBlurTexture(GLuint textureId);

private:
    GLint mBlurTextureSamplerHandler{0};
    /**
     * we do not have to release this texture
     * it should be released by its constructor
     */
    GLuint mBlurTexture{0};
};

#endif //ENGINE_HIGHPASSFILTER_H
