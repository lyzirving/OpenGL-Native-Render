//
// Created by liuyuzhou on 2021/9/18.
//
#ifndef ENGINE_OESFILTER_H
#define ENGINE_OESFILTER_H

#include "Static.h"
#include "BaseFilter.h"

class OesFilter : public BaseFilter {
public:

    OesFilter();
    ~OesFilter();
    void applyMatrix(const GLfloat* src, int size);
    void destroy() override;
    void initBuffer() override;
    void initFrameBuffer() override;
    void initHandler() override;
    void initTexture() override;
    void loadShader() override;
    GLint onDraw(GLint inputTextureId) override;
    void setCameraFaceFront(int faceFront);
    void setPreviewSize(GLint width, GLint height);

private:
    int mTransHandler{0};
    int mOesSamplerHandler{0};
    int mCameraFace{EngineUtil::CameraMetaData::LENS_FACING_BACK};
    GLuint* mOesFrameBuffer = nullptr;
    GLint mPreviewWidth{0};
    GLint mPreviewHeight{0};
};

#endif //ENGINE_OESFILTER_H
