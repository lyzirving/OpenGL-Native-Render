//
// Created by liuyuzhou on 2021/9/13.
//
#include "SharpenFilter.h"
#include "LogUtil.h"

#define TAG "SharpenFilter"


void SharpenFilter::adjust(int progress) {
    if (progress < 0) {
        progress = 0;
    } else if (progress > 100) {
        progress = 100;
    }
    if (progress < 50) {
        mSharpness = ((float)(50 - progress)) / 50 * -4;
    } else {
        mSharpness = ((float)(progress - 50)) / 50 * 4;
    }
    LogUtil::logI(TAG, {"adjust: progress = ", std::to_string(progress), ", sharpness = ", std::to_string(mSharpness)});
}

void SharpenFilter::initHandler() {
    BaseFilter::initHandler();
    mImgWidthHandler = glGetUniformLocation(mProgram, "uImgWidthFactor");
    mImgHeightHandler = glGetUniformLocation(mProgram, "uImgHeightFactor");
    mSharpnessHandler = glGetUniformLocation(mProgram, "uSharpness");
}

void SharpenFilter::loadShader() {
    auto gUtil = GlUtil::self();
    if (mVertexShader == nullptr) { mVertexShader = gUtil->readAssets("shader/sharpen_vertex_shader.glsl"); }
    if (mTextureShader == nullptr) { mTextureShader = gUtil->readAssets("shader/sharpen_fragment_shader.glsl"); }
}

GLint SharpenFilter::onDraw(GLint inputTextureId) {
    LogUtil::logI(TAG, {"onDraw: ", std::to_string(mSharpness)});
    glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
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

    glUniform1f(mImgWidthHandler, mWidthFactor);
    glUniform1f(mImgHeightHandler, mHeightFactor);
    glUniform1f(mSharpnessHandler, mSharpness);

    glDrawArrays(GL_TRIANGLES, 0, BaseFilter::DEFAULT_VERTEX_COUNT);

    glDisableVertexAttribArray(mVertexPosHandler);
    glDisableVertexAttribArray(mTextureCoordinateHandler);
    glBindTexture(GL_TEXTURE_2D, 0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    return inputTextureId;
}

void SharpenFilter::setOutputSize(GLint width, GLint height) {
    BaseFilter::setOutputSize(width, height);
    mWidthFactor = ((float)1) / mWidth;
    mHeightFactor = (float)1 / mHeight;
}

