//
// Created by liuyuzhou on 2021/9/9.
//
#include "MaskFilter.h"
#include "GlUtil.h"
#include "LogUtil.h"

#define TAG "MaskFilter"

void MaskFilter::buildMask() {
    if (mMaskPixel != nullptr) { delete[] mMaskPixel; }
    //format is RGBA_8888
    mMaskPixel = new unsigned char[mWidth * mHeight * 4];
    int maskStartX = (mWidth - mSourceWidth) / 2;
    int maskStartY = (mHeight - mSourceHeight) / 2;
    int maskEndX = maskStartX + mSourceWidth;
    int maskEndY = maskStartY + mSourceHeight;
    LogUtil::logI("MaskFilter", {"width = ", std::to_string(mWidth), ", height = ", std::to_string(mHeight), ", source width = ", std::to_string(mSourceWidth), ", source height = ", std::to_string(mSourceHeight)});
    LogUtil::logI("MaskFilter", {"buildMask: startX = ", std::to_string(maskStartX), ", endX = ", std::to_string(maskEndX), ", startY = ", std::to_string(maskStartY), ", endY = ", std::to_string(maskEndY)});
    for (int i = 0; i < mWidth; ++i) {
        for (int j = 0; j < mHeight; ++j) {
            if (i >= maskStartX && i <= maskEndX && j >= maskStartY && j <= maskEndY) {
                //mask is black
                mMaskPixel[j * mWidth + i] = 0;
                mMaskPixel[j * mWidth + i + 1] = 0;
                mMaskPixel[j * mWidth + i + 2] = 0;
                mMaskPixel[j * mWidth + i + 3] = 255;
            } else {
                //background is white
                mMaskPixel[j * mWidth + i] = 255;
                mMaskPixel[j * mWidth + i + 1] = 255;
                mMaskPixel[j * mWidth + i + 2] = 255;
                mMaskPixel[j * mWidth + i + 3] = 255;
            }
        }
    }
}

void MaskFilter::destroy() {
    BaseFilter::destroy();
    if (mMaskTextureId != 0) {
        glDeleteTextures(1, &mMaskTextureId);
        mMaskTextureId = 0;
    }
    if (mTextureId != 0) {
        glDeleteTextures(1, &mTextureId);
        mTextureId = 0;
    }
    if (mFrameBufferId != 0) {
        glDeleteFramebuffers(1, &mFrameBufferId);
        mFrameBufferId = 0;
    }
    if (mMaskPixel != nullptr) {
        delete[] mMaskPixel;
        mMaskPixel = nullptr;
    }
}

void MaskFilter::initFrameBuffer() {
    if (mFrameBufferId != 0) { glDeleteFramebuffers(1, &mFrameBufferId); }
    if (mTextureId != 0) { glDeleteTextures(1, &mTextureId); }

    glGenFramebuffers(1, &mFrameBufferId);
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

void MaskFilter::initHandler() {
    BaseFilter::initHandler();
    mMaskTextureSamplerHandler = glGetUniformLocation(mProgram, "sMaskSampler");
}

void MaskFilter::initTexture() {
    if (mMaskTextureId != 0) { glDeleteTextures(1, &mMaskTextureId); }

    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glGenTextures(1, &mMaskTextureId);
    glBindTexture(GL_TEXTURE_2D, mMaskTextureId);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_LUMINANCE, mWidth, mHeight, 0, GL_LUMINANCE, GL_UNSIGNED_BYTE, mMaskPixel);
    glBindTexture(GL_TEXTURE_2D, 0);
}

void MaskFilter::loadShader() {
    auto gUtil = GlUtil::self();
    if (mVertexShader == nullptr) { mVertexShader = gUtil->readAssets("shader/default_vertex_shader.glsl"); }
    if (mTextureShader == nullptr) { mTextureShader = gUtil->readAssets("shader/mask_fragment_shader.glsl"); }
}

GLint MaskFilter::onDraw(GLint inputTextureId) {
    glBindFramebuffer(GL_FRAMEBUFFER, mFrameBufferId);

    glUseProgram(mProgram);

    glBindBuffer(GL_ARRAY_BUFFER, mBufferId[0]);
    glVertexAttribPointer(mVertexPosHandler, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), nullptr);
    glEnableVertexAttribArray(mVertexPosHandler);

    glBindBuffer(GL_ARRAY_BUFFER, mBufferId[1]);
    glVertexAttribPointer(mTextureCoordinateHandler, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(GLfloat),nullptr);
    glEnableVertexAttribArray(mTextureCoordinateHandler);

    //source
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, inputTextureId);
    glUniform1i(mTextureSamplerHandler, 0);

    //mask
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, mMaskTextureId);
    glUniform1i(mMaskTextureSamplerHandler, 1);

    glDrawArrays(GL_TRIANGLES, 0, BaseFilter::DEFAULT_VERTEX_COUNT);

    glDisableVertexAttribArray(mVertexPosHandler);
    glDisableVertexAttribArray(mTextureCoordinateHandler);
    glBindTexture(GL_TEXTURE_2D, 0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    return mTextureId;
}

void MaskFilter::setSourceSize(int width, int height) {
    mSourceWidth = width;
    mSourceHeight = height;
}

