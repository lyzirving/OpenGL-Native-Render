//
// Created by liuyuzhou on 2021/9/29.
//
#ifndef ENGINE_DOWNLOADPIXELFILTER_H
#define ENGINE_DOWNLOADPIXELFILTER_H

#include "BaseFilter.h"

class DownloadPixelFilter : public BaseFilter {
public:

    GLuint getFrameBuffer() override;
    void initFrameBuffer() override;
    void initTexture() override;
    GLint onDraw(GLint inputTextureId) override;

private:
    GLuint* mDownloadFrameBuffer{nullptr};
};

#endif //ENGINE_DOWNLOADPIXELFILTER_H
