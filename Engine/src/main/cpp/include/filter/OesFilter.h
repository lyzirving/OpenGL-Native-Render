//
// Created by liuyuzhou on 2021/9/18.
//
#ifndef ENGINE_OESFILTER_H
#define ENGINE_OESFILTER_H

#include "BaseFilter.h"

class OesFilter : public BaseFilter {
public:

    OesFilter();
    ~OesFilter();
    void applyMatrix(const GLfloat* src, int size);
    void destroy() override;
    void initFrameBuffer() override;
    void initHandler() override;
    void initTexture() override;
    void loadShader() override;
    GLint onDraw(GLint inputTextureId) override;

private:
    int mTransHandler{0};
    int mOesSamplerHandler{0};
    GLuint* mOesFrameBuffer = nullptr;
};

#endif //ENGINE_OESFILTER_H
