//
// Created by liuyuzhou on 2021/10/22.
//

#ifndef ENGINE_MODELFILTER_H
#define ENGINE_MODELFILTER_H

#include "BaseFilter.h"
#include "ObjGroup.h"

class ModelFilter : public BaseFilter {
public:
    ModelFilter();

    ~ModelFilter();

    void destroy() override;

    void initBuffer() override;

    void initHandler() override;

    void initTexture() override;

    void loadShader() override;

    GLint onDraw(GLint inputTextureId) override;

    void onResume() override;

    void onPause() override;

    void postInit() override;

    void setObjGroup(ObjGroup* group);

    void setOutputSize(GLint width, GLint height) override;

private:
    GLfloat *mMvpMatrix{nullptr};
    GLfloat *mProjectionMatrix{nullptr};
    GLfloat *mViewMatrix{nullptr};
    /**
     * it is a 4x4 matrix, and it is ordered as below
     * { m0  m4  m8   m12
     *   m1  m5  m9   m13
     *   m2  m6  m10  m14
     *   m3  m7  m11  m15 }
     *   column(m0 m1 m2 m3) is x-axis direction of model coordinate in world coordinate
     *   column(m4 m5 m6 m7) is y-axis direction of model coordinate in world coordinate
     *   column(m8 m9 m10 m11) is z-axis direction of model coordinate in world coordinate
     *   column(m12 m13 m14 m15) is origin position of model coordinate in world coordinate
     */
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
    GLuint* mModelTexture{nullptr};

    GLfloat (*mAmbient)[3]{nullptr};
    GLfloat (*mDiffuse)[3]{nullptr};

};

#endif //ENGINE_MODELFILTER_H
