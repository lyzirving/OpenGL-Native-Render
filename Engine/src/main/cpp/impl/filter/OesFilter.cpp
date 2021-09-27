//
// Created by liuyuzhou on 2021/9/18.
//
#include "OesFilter.h"
#include "MatrixUtil.h"
#include "GlUtil.h"
#include "LogUtil.h"

/**
 * this file must be included after <GLES2/gl2.h>
 * BaseFilter.h contains file <GLES2/gl2.h>
 */
#include <GLES2/gl2ext.h>

#define TAG "OesFilter"

void OesFilter::calculateMatrix() {
    if (mMatrix == nullptr) { mMatrix = new GLfloat[16]; }
    auto* tmpLhsM = new GLfloat[16];
    auto* tmpResultM = new GLfloat[16];
    MatrixUtil::setIdentityM(mMatrix, 0);
    MatrixUtil::setIdentityM(tmpLhsM, 0);
    MatrixUtil::setIdentityM(tmpResultM, 0);

    //get rotation matrix
    if (mCameraFace == render::CameraMetaData::LENS_FACING_FRONT) {
        MatrixUtil::flip(mMatrix, false, true);
        MatrixUtil::setRotate(tmpLhsM, 90, 0, 0, 1);
    } else if (mCameraFace == render::CameraMetaData::LENS_FACING_BACK) {
        MatrixUtil::setRotate(tmpLhsM, 270, 0, 0, 1);
    }
    MatrixUtil::multiplyMM(tmpResultM, tmpLhsM, mMatrix);
    MatrixUtil::setIdentityM(tmpLhsM, 0);
    MatrixUtil::setIdentityM(mMatrix, 0);

    GLfloat previewRatio = ((GLfloat)mPreviewWidth) / ((GLfloat)mPreviewHeight);
    GLfloat viewRatio = ((GLfloat)mWidth) / ((GLfloat)mHeight);
    if (previewRatio > viewRatio) {
        MatrixUtil::scaleM(tmpLhsM, 0, viewRatio / previewRatio, 1, 1);
    }  else if (previewRatio < viewRatio) {
        MatrixUtil::scaleM(tmpLhsM, 0, 1, previewRatio / viewRatio,1);
    }
    MatrixUtil::multiplyMM(mMatrix, tmpLhsM, tmpResultM);
    MatrixUtil::setIdentityM(tmpLhsM, 0);
    MatrixUtil::setIdentityM(tmpResultM, 0);

    delete[] tmpLhsM;
    delete[] tmpResultM;
}

void OesFilter::destroy() {
    BaseFilter::destroy();
    if (mOesFrameBuffer != nullptr) {
        glDeleteFramebuffers(1, mOesFrameBuffer);
        delete[] mOesFrameBuffer;
        mOesFrameBuffer = nullptr;
    }
    delete[] mMatrix;
}

GLuint OesFilter::getOesFrameBuffer() {
    if (mOesFrameBuffer == nullptr) { return GL_NONE; }
    return mOesFrameBuffer[0];
}

void OesFilter::init() {
    if (!mInitialized) { LogUtil::logI(TAG, {"init: begin to initialized filter"}); }
    BaseFilter::init();
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

GLint OesFilter::onDraw(GLint oesInputTexture) {
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
    glBindTexture(GL_TEXTURE_EXTERNAL_OES, oesInputTexture);
    glUniform1i(mOesSamplerHandler, 0);

    glDrawArrays(GL_TRIANGLES, 0, BaseFilter::DEFAULT_VERTEX_COUNT);

    glDisableVertexAttribArray(mVertexPosHandler);
    glDisableVertexAttribArray(mTextureCoordinateHandler);
    glBindTexture(GL_TEXTURE_2D, 0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    return mTextureId;
}

void OesFilter::onPause() {
    BaseFilter::onPause();
    if (mOesFrameBuffer != nullptr) {
        glDeleteFramebuffers(1, mOesFrameBuffer);
        delete[] mOesFrameBuffer;
        mOesFrameBuffer = nullptr;
    }
    delete[] mMatrix;
}

void OesFilter::setCameraFaceFront(int faceFront) {
    mCameraFace = faceFront;
}

void OesFilter::setPreviewSize(GLint width, GLint height) {
    mPreviewWidth = width;
    mPreviewHeight = height;
}

