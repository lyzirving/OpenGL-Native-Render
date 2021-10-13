//
// Created by liuyuzhou on 2021/10/8.
//
#include "FaceLiftFilter.h"
#include "GlUtil.h"
#include "MatrixUtil.h"
#include "LogUtil.h"

#define TAG "FaceLiftFilter"

FaceLiftFilter::FaceLiftFilter() {
    mLhsDstPt = new GLfloat[2]{0, 0};
    mLhsCtrlPt = new GLfloat[2]{0, 0};
    mRhsDstPt = new GLfloat[2]{0, 0};
    mRhsCtrlPt = new GLfloat[2]{0, 0};
    mMatrix = new GLfloat[16];
    MatrixUtil::setIdentityM(mMatrix, 0);
    pthread_mutex_init(&mMutex, nullptr);
}

FaceLiftFilter::~FaceLiftFilter() {
    delete[] mLhsCtrlPt;
    delete[] mLhsDstPt;
    delete[] mRhsCtrlPt;
    delete[] mRhsDstPt;
    delete[] mMatrix;
    pthread_mutex_destroy(&mMutex);
}

void FaceLiftFilter::adjust(int progress) {
    if (progress < 0) {
        progress = 0;
    } else if (progress > 100) {
        progress = 100;
    }
    mIntensity = ((GLfloat)(progress)) / 100 * 3;
    LogUtil::logI(TAG, {"adjust: intensity = ", std::to_string(mIntensity)});
}

void FaceLiftFilter::destroy() {
    BaseFilter::destroy();
    if (mFrameBufferId != 0) {
        glDeleteFramebuffers(1, &mFrameBufferId);
        mFrameBufferId = 0;
    }
}

void FaceLiftFilter::flip(bool horizontal, bool vertical) {
    MatrixUtil::flip(mMatrix, horizontal, vertical);
}

void FaceLiftFilter::loadShader() {
    auto gUtil = GlUtil::self();
    if (mVertexShader == nullptr) { mVertexShader = gUtil->readAssets("shader/trans_vertex_shader.glsl"); }
    if (mTextureShader == nullptr) { mTextureShader = gUtil->readAssets("shader/face_lift_fragment_shader.glsl"); }
}

void FaceLiftFilter::initFrameBuffer() {
    if (mFrameBufferId != 0) { glDeleteFramebuffers(1, &mFrameBufferId); }
    glGenFramebuffers(1, &mFrameBufferId);
}

void FaceLiftFilter::initTexture() {
    if (mTextureId != 0) { glDeleteTextures(1, &mTextureId); }
    glGenTextures(1, &mTextureId);

    glBindTexture(GL_TEXTURE_2D, mTextureId);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, mWidth, mHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE,nullptr);

    glBindFramebuffer(GL_FRAMEBUFFER, mFrameBufferId);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, mTextureId, 0);

    glBindTexture(GL_TEXTURE_2D, 0);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void FaceLiftFilter::initHandler() {
    BaseFilter::initHandler();
    mWidthHandler = glGetUniformLocation(mProgram, "uImgWidth");
    mHeightHandler = glGetUniformLocation(mProgram, "uImgHeight");
    mIntensityHandler = glGetUniformLocation(mProgram, "uIntensity");
    mLhsDstHandler = glGetUniformLocation(mProgram, "uLhsDstPt");
    mLhsCtrlHandler = glGetUniformLocation(mProgram, "uLhsCtrlPt");
    mRhsDstHandler = glGetUniformLocation(mProgram, "uRhsDstPt");
    mRhsCtrlHandler = glGetUniformLocation(mProgram, "uRhsCtrlPt");
    mTransHandler = glGetUniformLocation(mProgram, "uMatrix");
}

GLint FaceLiftFilter::onDraw(GLint inputTextureId) {
    glBindFramebuffer(GL_FRAMEBUFFER, mFrameBufferId);

    glUseProgram(mProgram);

    glBindBuffer(GL_ARRAY_BUFFER, mBufferId[0]);
    glVertexAttribPointer(mVertexPosHandler, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), nullptr);
    glEnableVertexAttribArray(mVertexPosHandler);

    glBindBuffer(GL_ARRAY_BUFFER, mBufferId[1]);
    glVertexAttribPointer(mTextureCoordinateHandler, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(GLfloat),nullptr);
    glEnableVertexAttribArray(mTextureCoordinateHandler);

    glUniform1f(mWidthHandler, (GLfloat)mWidth);
    glUniform1f(mHeightHandler, (GLfloat)mHeight);
    glUniform1f(mIntensityHandler, mIntensity);
    glUniform2fv(mLhsDstHandler, 1, mLhsDstPt);
    glUniform2fv(mLhsCtrlHandler, 1, mLhsCtrlPt);
    glUniform2fv(mRhsDstHandler, 1, mRhsDstPt);
    glUniform2fv(mRhsCtrlHandler, 1, mRhsCtrlPt);

    glUniformMatrix4fv(mTransHandler, 1, false, mMatrix);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, inputTextureId);
    glUniform1i(mTextureSamplerHandler, 0);

    glDrawArrays(GL_TRIANGLES, 0, BaseFilter::DEFAULT_VERTEX_COUNT);

    glDisableVertexAttribArray(mVertexPosHandler);
    glDisableVertexAttribArray(mTextureCoordinateHandler);
    MatrixUtil::setIdentityM(mMatrix, 0);
    glBindTexture(GL_TEXTURE_2D, 0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    return mTextureId;
}

void FaceLiftFilter::onPause() {
    BaseFilter::onPause();
    if (mFrameBufferId != 0) {
        glDeleteFramebuffers(1, &mFrameBufferId);
        mFrameBufferId = 0;
    }
}

bool FaceLiftFilter::pointValid() {
    pthread_mutex_lock(&mMutex);
    bool result = !((mLhsDstPt[0] == 0 && mLhsDstPt[1] == 0)
                    || (mLhsCtrlPt[0] == 0 && mLhsCtrlPt[1] == 0)
                    || (mRhsDstPt[0] == 0 && mRhsDstPt[1] == 0)
                    || (mRhsCtrlPt[0] == 0 && mRhsCtrlPt[1] == 0));
    pthread_mutex_unlock(&mMutex);
    return result;
}

void FaceLiftFilter::setPts(Point *lhsDst, Point *lhsCtrl, Point *rhsDst, Point *rhsCtrl) {
    pthread_mutex_lock(&mMutex);
    setLhsDstPt(lhsDst->x, lhsDst->y);
    setLhsCtrlPt(lhsCtrl->x, lhsCtrl->y);
    setRhsDstPt(rhsDst->x, rhsDst->y);
    setRhsCtrlPt(rhsCtrl->x, rhsCtrl->y);
    pthread_mutex_unlock(&mMutex);
}

void FaceLiftFilter::setLhsDstPt(GLfloat x, GLfloat y) {
    mLhsDstPt[0] = x;
    mLhsDstPt[1] = y;
}

void FaceLiftFilter::setLhsCtrlPt(GLfloat x, GLfloat y) {
    mLhsCtrlPt[0] = x;
    mLhsCtrlPt[1] = y;
}

void FaceLiftFilter::setRhsDstPt(GLfloat x, GLfloat y) {
    mRhsDstPt[0] = x;
    mRhsDstPt[1] = y;
}

void FaceLiftFilter::setRhsCtrlPt(GLfloat x, GLfloat y) {
    mRhsCtrlPt[0] = x;
    mRhsCtrlPt[1] = y;
}
