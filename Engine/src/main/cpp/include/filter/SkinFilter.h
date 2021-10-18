//
// Created by liuyuzhou on 2021/10/15.
//
#ifndef ENGINE_SKINFILTER_H
#define ENGINE_SKINFILTER_H

#include "BaseFilter.h"

class SkinFilter : public BaseFilter {
public:

    void adjustProperty(const char *property, int progress) override;
    void destroy() override;
    void initHandler() override;
    void initTexture() override;
    void loadShader() override;
    GLint onDraw(GLint inputTextureId) override;
    void onPause() override;

private:
    GLuint mGrayTexture{0};
    GLuint mLookupTexture{0};

    GLint mGrayTextureSampler{0};
    GLint mLookupTextureSampler{0};

    GLint mLevelRangeHandler{0};
    GLint mLevelBlackHandler{0};
    GLint mAlphaHandler{0};

    GLfloat mLevelRangeIne{1.040816};
    GLfloat mLevelBlack{0.01960784};
    GLfloat mAlpha{0.5};
};

#endif //ENGINE_SKINFILTER_H
