//
// Created by liuyuzhou on 2021/9/9.
//
#ifndef ENGINE_MASKFILTER_H
#define ENGINE_MASKFILTER_H

#include "BaseFilter.h"

class MaskFilter : public BaseFilter {
public:

    void buildMask();
    void destroy() override;
    void initHandler() override;
    void initTexture() override;
    void initFrameBuffer() override;
    void loadShader() override;
    GLint onDraw(GLint inputTextureId) override;
    void onPause() override;
    void setSourceSize(int width, int height);

private:
    int mSourceWidth{0};
    int mSourceHeight{0};
    unsigned char* mMaskPixel = nullptr;
    GLint mMaskTextureSamplerHandler{0};
    GLuint mMaskTextureId{0};
    GLuint mFrameBufferId{0};
};

#endif //ENGINE_MASKFILTER_H
