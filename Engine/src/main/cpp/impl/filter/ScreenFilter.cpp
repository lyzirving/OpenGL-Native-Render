//
// Created by liuyuzhou on 2021/9/8.
//
#include "ScreenFilter.h"
#include "MatrixUtil.h"
#include "LogUtil.h"

#define TAG "ScreenFilter"

ScreenFilter::ScreenFilter() {
    mMatrix = new GLfloat[16];
}

void ScreenFilter::flip(bool horizontal, bool vertical) {
    MatrixUtil::flip(mMatrix, horizontal, vertical);
}

void ScreenFilter::init() {
    BaseFilter::init();
    LogUtil::logI(TAG, {"init: ", std::to_string(mInitialized)});
}

void ScreenFilter::initHandler() {
    BaseFilter::initHandler();
    mTransHandler = glGetUniformLocation(mProgram, "uMatrix");
    MatrixUtil::setIdentityM(mMatrix, 0);
}

void ScreenFilter::loadShader() {
    auto gUtil = GlUtil::self();
    if (mVertexShader == nullptr) { mVertexShader = gUtil->readAssets("shader/trans_vertex_shader.glsl"); }
    if (mTextureShader == nullptr) { mTextureShader = gUtil->readAssets("shader/default_fragment_shader.glsl"); }
}

GLint ScreenFilter::onDraw(GLint inputTextureId) {
    //0 is screen frame buffer id
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
    glUseProgram(mProgram);

    glBindBuffer(GL_ARRAY_BUFFER, mBufferId[0]);
    glVertexAttribPointer(mVertexPosHandler, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), nullptr);
    glEnableVertexAttribArray(mVertexPosHandler);

    glBindBuffer(GL_ARRAY_BUFFER, mBufferId[1]);
    glVertexAttribPointer(mTextureCoordinateHandler, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(GLfloat),nullptr);
    glEnableVertexAttribArray(mTextureCoordinateHandler);

    glUniformMatrix4fv(mTransHandler, 1, false, mMatrix);

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

