//
// Created by liuyuzhou on 2021/10/8.
//
#include "PlaceHolderFilter.h"
#include "GlUtil.h"
#include "MatrixUtil.h"
#include "LogUtil.h"

#define TAG "PlaceHolderFilter"

PlaceHolderFilter::PlaceHolderFilter() {
    mMatrix = new GLfloat[16];
    MatrixUtil::setIdentityM(mMatrix, 0);
}

PlaceHolderFilter::~PlaceHolderFilter() {
    delete[] mMatrix;
}

void PlaceHolderFilter::destroy() {
    BaseFilter::destroy();
    if (mFrameBufferId != 0) {
        glDeleteFramebuffers(1, &mFrameBufferId);
        mFrameBufferId = 0;
    }
}

void PlaceHolderFilter::flip(bool horizontal, bool vertical) {
    MatrixUtil::flip(mMatrix, horizontal, vertical);
}

void PlaceHolderFilter::initFrameBuffer() {
    if (mFrameBufferId != 0) { glDeleteFramebuffers(1, &mFrameBufferId); }
    glGenFramebuffers(1, &mFrameBufferId);
}

void PlaceHolderFilter::initHandler() {
    BaseFilter::initHandler();
    mTransHandler = glGetUniformLocation(mProgram, "uMatrix");
}

void PlaceHolderFilter::initTexture() {
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

void PlaceHolderFilter::loadShader() {
    auto gUtil = GlUtil::self();
    if (mVertexShader == nullptr) { mVertexShader = gUtil->readAssets("shader/trans_vertex_shader.glsl"); }
    if (mTextureShader == nullptr) { mTextureShader = gUtil->readAssets("shader/default_fragment_shader.glsl"); }
}

GLint PlaceHolderFilter::onDraw(GLint inputTextureId) {
    //0 is screen frame buffer id
    glBindFramebuffer(GL_FRAMEBUFFER, mFrameBufferId);
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
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    MatrixUtil::setIdentityM(mMatrix, 0);

    return mTextureId;
}

void PlaceHolderFilter::onPause() {
    BaseFilter::onPause();
    if (mFrameBufferId != 0) {
        glDeleteFramebuffers(1, &mFrameBufferId);
        mFrameBufferId = 0;
    }
}
