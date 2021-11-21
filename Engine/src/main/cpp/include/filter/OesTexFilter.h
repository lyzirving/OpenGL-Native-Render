//
// Created by lyzirving on 2021/11/21.
//
#ifndef ENGINE_OESTEXFILTER_H
#define ENGINE_OESTEXFILTER_H

#include "BaseFilter.h"

class OesTexFilter : public BaseFilter {
private:
    GLfloat mTexMatrix[16]{};
    GLint mOesTexMatrixHandler{0};
    GLint mOesTextureSamplerHandler{0};
public:
    OesTexFilter();
    ~OesTexFilter();

    void init() override;

    void initHandler() override;

    void loadShader() override;

    GLint onDraw(GLint inputTextureId) override;

    void setTexMatrix(const GLfloat* matrix);

};

#endif //ENGINE_OESTEXFILTER_H
