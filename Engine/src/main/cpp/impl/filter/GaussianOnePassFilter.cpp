//
// Created by liuyuzhou on 2021/9/14.
//
#include "GaussianOnePassFilter.h"
#include "GlUtil.h"
#include "MatrixUtil.h"
#include "LogUtil.h"

#define TAG "GaussianOnePassFilter"

GaussianOnePassFilter::GaussianOnePassFilter(Orientation orientation) {
    mMatrix = new GLfloat[16];
    MatrixUtil::setIdentityM(mMatrix, 0);
    mOrientation = orientation;
}

GaussianOnePassFilter::~GaussianOnePassFilter() {
    delete[] mMatrix;
}

void GaussianOnePassFilter::adjustHorBlur(float blurSize) {
    mHorBlurSize = blurSize;
}

void GaussianOnePassFilter::adjustVerBlur(float blurSize) {
    mVerBlurSize = blurSize;
}

void GaussianOnePassFilter::init() {
    if (!mInitialized) { LogUtil::logI(TAG, {"init: begin to initialized filter"}); }
    BaseFilter::init();
}

void GaussianOnePassFilter::flip(bool horFlip, bool verFlip) {
    MatrixUtil::flip(mMatrix, horFlip, verFlip);
}

void GaussianOnePassFilter::initHandler() {
    BaseFilter::initHandler();
    mTextureWidthOffsetHandler = glGetUniformLocation(mProgram, "uTextureWidthOffset");
    mTextureHeightOffsetHandler = glGetUniformLocation(mProgram, "uTextureHeightOffset");
    mMatrixHandler = glGetUniformLocation(mProgram, "uMatrix");
}

void GaussianOnePassFilter::loadShader() {
    auto gUtil = GlUtil::self();
    if (mVertexShader == nullptr) { mVertexShader = gUtil->readAssets("shader/gaussian_vertex_shader.glsl"); }
    if (mTextureShader == nullptr) { mTextureShader = gUtil->readAssets("shader/gaussian_fragment_shader.glsl"); }
}

GLint GaussianOnePassFilter::onDraw(GLint inputTextureId) {
    glUseProgram(mProgram);

    glBindBuffer(GL_ARRAY_BUFFER, mBufferId[0]);
    glVertexAttribPointer(mVertexPosHandler, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), nullptr);
    glEnableVertexAttribArray(mVertexPosHandler);

    glBindBuffer(GL_ARRAY_BUFFER, mBufferId[1]);
    glVertexAttribPointer(mTextureCoordinateHandler, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(GLfloat),nullptr);
    glEnableVertexAttribArray(mTextureCoordinateHandler);

    glUniform1f(mTextureWidthOffsetHandler, (mOrientation == Orientation::HORIZONTAL) ? (mHorBlurSize / mWidth) : 0);
    glUniform1f(mTextureHeightOffsetHandler, (mOrientation == Orientation::VERTICAL) ? (mVerBlurSize / mHeight) : 0);
    glUniformMatrix4fv(mMatrixHandler, 1, false, mMatrix);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, inputTextureId);
    glUniform1i(mTextureSamplerHandler, 0);

    glDrawArrays(GL_TRIANGLES, 0, BaseFilter::DEFAULT_VERTEX_COUNT);

    glDisableVertexAttribArray(mVertexPosHandler);
    glDisableVertexAttribArray(mTextureCoordinateHandler);
    glBindTexture(GL_TEXTURE_2D, 0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    MatrixUtil::setIdentityM(mMatrix, 0);

    return inputTextureId;
}

