//
// Created by liuyuzhou on 2021/9/18.
//
#include "OesFilter.h"
#include "MatrixUtil.h"

OesFilter::OesFilter() {
    mMatrix = new GLfloat[16];
    MatrixUtil::setIdentityM(mMatrix, 0);
}

OesFilter::~OesFilter() {
    delete[] mMatrix;
}

void OesFilter::applyMatrix(const GLfloat *src, int size) {
    if (mMatrix == nullptr) { mMatrix = new GLfloat[16]; }
    for (int i = 0; i < size; ++i) { mMatrix[i] = src[i]; }
}

void OesFilter::destroy() {
    BaseFilter::destroy();
    if (mOesFrameBuffer != nullptr) {
        glDeleteFramebuffers(1, mOesFrameBuffer);
        delete[] mOesFrameBuffer;
        mOesFrameBuffer = nullptr;
    }
}

void OesFilter::initHandler() {
    mVertexPosHandler = glGetAttribLocation(mProgram, "aVertexPos");
    mTextureCoordinateHandler = glGetAttribLocation(mProgram, "aTextureCoordinate");
    mOesSamplerHandler = glGetUniformLocation(mProgram, "sOesTextureSampler");
    mTransHandler = glGetUniformLocation(mProgram, "uMatrix");
}

void OesFilter::initFrameBuffer() {
    if (mOesFrameBuffer != nullptr) {
        glDeleteFramebuffers(1, mOesFrameBuffer);
        delete[] mOesFrameBuffer;
        mOesFrameBuffer = nullptr;
    }
    mOesFrameBuffer = new GLuint[1];
    glGenFramebuffers(1, mOesFrameBuffer);
}

void OesFilter::initTexture() {
    if (mTextureId != 0) {
        glDeleteTextures(1, &mTextureId);
        mTextureId = 0;
    }
    glGenTextures(1, &mTextureId);
    glBindTexture(GL_TEXTURE_2D, mTextureId);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, mWidth, mHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE,nullptr);

    glBindFramebuffer(GL_FRAMEBUFFER, mOesFrameBuffer[0]);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, mTextureId, 0);

    glBindTexture(GL_TEXTURE_2D, 0);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void OesFilter::loadShader() {
    auto gUtil = GlUtil::self();
    if (mVertexShader == nullptr) { mVertexShader = gUtil->readAssets("shader/trans_vertex_shader.glsl"); }
    if (mTextureShader == nullptr) { mTextureShader = gUtil->readAssets("shader/oes_fragment_shader.glsl"); }
}

GLint OesFilter::onDraw(GLint inputTextureId) {
    glBindFramebuffer(GL_FRAMEBUFFER, mOesFrameBuffer[0]);
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
    glUniform1i(mOesSamplerHandler, 0);

    glDrawArrays(GL_TRIANGLES, 0, BaseFilter::DEFAULT_VERTEX_COUNT);

    glDisableVertexAttribArray(mVertexPosHandler);
    glDisableVertexAttribArray(mTextureCoordinateHandler);
    glBindTexture(GL_TEXTURE_2D, 0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    MatrixUtil::setIdentityM(mMatrix, 0);

    return mTextureId;
}

