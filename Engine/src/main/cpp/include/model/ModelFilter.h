//
// Created by liuyuzhou on 2021/10/22.
//

#ifndef ENGINE_MODELFILTER_H
#define ENGINE_MODELFILTER_H

#include "BaseFilter.h"

class ModelFilter : public BaseFilter {
public:
    void initBuffer() override;

    void initHandler() override;

    void loadShader() override;

private:
    GLint mVertexNormalHandler{0};
    GLint mMVPMatrixHandler{0};
    GLint mMMatrixHandler{0};
    GLint mLightLocationHandler{0};
    GLint mAmbientHandler{0};
    GLint mDiffuseHandler{0};

    GLfloat *mVertexNormal = nullptr;
    GLuint mVertexNormalBufferId{0};
};

#endif //ENGINE_MODELFILTER_H
