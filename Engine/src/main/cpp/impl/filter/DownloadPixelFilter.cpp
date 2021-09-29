//
// Created by liuyuzhou on 2021/9/29.
//
#include "DownloadPixelFilter.h"
#include "GlUtil.h"
#include "MatrixUtil.h"
#include "LogUtil.h"

#define TAG "DownloadPixelFilter"

GLuint DownloadPixelFilter::getFrameBuffer() {
    return mDownloadFrameBuffer != nullptr ? mDownloadFrameBuffer[0] : 0;
}

void DownloadPixelFilter::initFrameBuffer() {
    if (mDownloadFrameBuffer != nullptr) {
        glDeleteFramebuffers(1, mDownloadFrameBuffer);
        delete[] mDownloadFrameBuffer;
        mDownloadFrameBuffer = nullptr;
    }
    mDownloadFrameBuffer = new GLuint[1];
    glGenFramebuffers(1, mDownloadFrameBuffer);
}

void DownloadPixelFilter::initTexture() {
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

    glBindFramebuffer(GL_FRAMEBUFFER, mDownloadFrameBuffer[0]);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, mTextureId, 0);

    glBindTexture(GL_TEXTURE_2D, 0);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

GLint DownloadPixelFilter::onDraw(GLint inputTextureId) {
    glBindFramebuffer(GL_FRAMEBUFFER, mDownloadFrameBuffer[0]);
    glUseProgram(mProgram);

    glBindBuffer(GL_ARRAY_BUFFER, mBufferId[0]);
    glVertexAttribPointer(mVertexPosHandler, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), nullptr);
    glEnableVertexAttribArray(mVertexPosHandler);
    glBindBuffer(GL_ARRAY_BUFFER, mBufferId[1]);
    glVertexAttribPointer(mTextureCoordinateHandler, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(GLfloat),nullptr);
    glEnableVertexAttribArray(mTextureCoordinateHandler);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, inputTextureId);
    glUniform1i(mTextureSamplerHandler, 0);

    glDrawArrays(GL_TRIANGLES, 0, BaseFilter::DEFAULT_VERTEX_COUNT);

    glDisableVertexAttribArray(mVertexPosHandler);
    glDisableVertexAttribArray(mTextureCoordinateHandler);
    glBindTexture(GL_TEXTURE_2D, 0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    return mTextureId;
}

