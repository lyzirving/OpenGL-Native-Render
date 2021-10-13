//
// Created by liuyuzhou on 2021/10/8.
//
#ifndef RENDERDEMO_FACELIFTFILTER_H
#define RENDERDEMO_FACELIFTFILTER_H

#include <pthread.h>
#include "BaseFilter.h"
#include "Point.h"

/**
 * core theory is to calculate the offset of pixel
 * move the pixel from ctrl pt to dest pt
 * intensity: 0 is default with no change, 3 is the maximum
 */
class FaceLiftFilter : public BaseFilter {
public:

    FaceLiftFilter();
    ~FaceLiftFilter();
    void adjust(int progress) override;
    void destroy() override;
    void flip(bool horizontal, bool vertical);
    void initFrameBuffer() override;
    void initHandler() override;
    void loadShader() override;
    void initTexture() override;
    GLint onDraw(GLint inputTextureId) override;
    void onPause() override;
    bool pointValid();
    void setPts(Point* lhsDst, Point* lhsCtrl, Point* rhsDst, Point* rhsCtrl);
    void setLhsDstPt(GLfloat x, GLfloat y);
    void setLhsCtrlPt(GLfloat x, GLfloat y);
    void setRhsDstPt(GLfloat x, GLfloat y);
    void setRhsCtrlPt(GLfloat x, GLfloat y);

private:
    GLuint mFrameBufferId{0};

    GLint mWidthHandler{0};
    GLint mHeightHandler{0};
    GLint mIntensityHandler{0};
    GLint mLhsDstHandler{0};
    GLint mLhsCtrlHandler{0};
    GLint mRhsDstHandler{0};
    GLint mRhsCtrlHandler{0};

    GLfloat* mLhsDstPt{nullptr};
    GLfloat* mLhsCtrlPt{nullptr};

    GLfloat* mRhsDstPt{nullptr};
    GLfloat* mRhsCtrlPt{nullptr};

    GLfloat mIntensity{0.5};
    pthread_mutex_t mMutex{};
    int mTransHandler{0};
};

#endif //RENDERDEMO_FACELIFTFILTER_H
