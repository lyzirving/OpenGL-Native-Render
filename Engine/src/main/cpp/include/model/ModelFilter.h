//
// Created by liuyuzhou on 2021/10/22.
//

#ifndef ENGINE_MODELFILTER_H
#define ENGINE_MODELFILTER_H

#include "BaseFilter.h"
#include "ObjGroup.h"

class ModelFilter : public BaseFilter {
public:
    void initBuffer() override;

    void initHandler() override;

    void initFrameBuffer() override;

    void initTexture() override;

    void loadShader() override;

    GLint onDraw(GLint inputTextureId) override;

    void postInit() override;

    void setObjGroup(ObjGroup* group);

private:
    GLfloat *mMvpMatrix{nullptr};
    GLfloat *mProjectionMatrix{nullptr};
    GLfloat *mModelMatrix{nullptr};

    GLint mVertexNormalHandler{0};
    GLint mMVPMatrixHandler{0};
    GLint mMMatrixHandler{0};
    GLint mLightLocationHandler{0};
    GLint mAmbientHandler{0};
    GLint mDiffuseHandler{0};

    ObjGroup* mObjGroup{nullptr};
    GLuint* mVertexBufferId{nullptr};
    GLuint* mTextureCoordBufferId{nullptr};
    GLuint* mVertexNormalBufferId{nullptr};
    GLfloat (*mAmbient)[3]{nullptr};
    GLfloat (*mDiffuse)[3]{nullptr};
    GLuint* mModelTexture{nullptr};
    GLuint mFrameBufferId{0};
};

#endif //ENGINE_MODELFILTER_H
