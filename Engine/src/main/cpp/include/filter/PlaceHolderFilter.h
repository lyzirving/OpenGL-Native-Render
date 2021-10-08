//
// Created by liuyuzhou on 2021/10/8.
//
#ifndef ENGINE_PLACEHOLDERFILTER_H
#define ENGINE_PLACEHOLDERFILTER_H

#include "BaseFilter.h"

class PlaceHolderFilter : public BaseFilter {
public:
    PlaceHolderFilter();
    ~PlaceHolderFilter();
    void destroy() override;
    void flip(bool horizontal, bool vertical);
    void initFrameBuffer() override;
    void initHandler() override;
    void initTexture() override;
    void loadShader() override;
    GLint onDraw(GLint inputTextureId) override;
    void onPause() override;

private:
    GLuint mFrameBufferId{0};
    int mTransHandler{0};
};

#endif //ENGINE_PLACEHOLDERFILTER_H
