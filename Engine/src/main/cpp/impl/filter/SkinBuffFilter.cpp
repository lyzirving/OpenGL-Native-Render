//
// Created by liuyuzhou on 2021/10/15.
//
#include "SkinBuffFilter.h"
#include "Common.h"
#include "GlUtil.h"
#include "MatrixUtil.h"
#include "LogUtil.h"

#define TAG "SkinBuffFilter"

void SkinBuffFilter::adjustProperty(const char *property, int progress) {
    if (property == nullptr || std::strlen(property) == 0) {
        LogUtil::logI(TAG, {"adjustProperty: filter prop is invalid"});
    } else if (std::strcmp(render::FILTER_PROP_SKIN_BUFF, property) == 0) {
        mSkinBuffIntensity = (float)progress / 100 * 0.4;
        LogUtil::logI(TAG, {"adjustProperty: skin buff, intensity = ", std::to_string(mSkinBuffIntensity), ", progress = ", std::to_string(progress)});
    } else {
        LogUtil::logI(TAG, {"adjustProperty: do not handle property ", property});
    }
}

void SkinBuffFilter::initHandler() {
    BaseFilter::initHandler();
    mBlurTextureSamplerHandler = glGetUniformLocation(mProgram, "sBlurTextureSampler");
    mHighPassBlurTextureSamplerHandler = glGetUniformLocation(mProgram, "sHighPassBlurTextureSampler");
    mSkinBuffIntensityHandler = glGetUniformLocation(mProgram, "uSkinBuffIntensity");

    mImgWidthHandler = glGetUniformLocation(mProgram, "uImgWidthFactor");
    mImgHeightHandler = glGetUniformLocation(mProgram, "uImgHeightFactor");
    mSharpnessHandler = glGetUniformLocation(mProgram, "uSharpness");
}

void SkinBuffFilter::loadShader() {
    auto gUtil = GlUtil::self();
    if (mVertexShader == nullptr) { mVertexShader = gUtil->readAssets("shader/skin_buff_vertex_shader.glsl"); }
    if (mTextureShader == nullptr) { mTextureShader = gUtil->readAssets("shader/skin_buff_fragment_shader.glsl"); }
}

GLint SkinBuffFilter::onDraw(GLint inputTextureId) {
    glUseProgram(mProgram);

    glBindBuffer(GL_ARRAY_BUFFER, mBufferId[0]);
    glVertexAttribPointer(mVertexPosHandler, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), nullptr);
    glEnableVertexAttribArray(mVertexPosHandler);

    glBindBuffer(GL_ARRAY_BUFFER, mBufferId[1]);
    glVertexAttribPointer(mTextureCoordinateHandler, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(GLfloat),nullptr);
    glEnableVertexAttribArray(mTextureCoordinateHandler);

    glUniform1f(mSkinBuffIntensityHandler, mSkinBuffIntensity);

    glUniform1f(mImgWidthHandler, ((float)1) / mWidth);
    glUniform1f(mImgHeightHandler, ((float)1) / mHeight);
    glUniform1f(mSharpnessHandler, mSharpness);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, inputTextureId);
    glUniform1i(mTextureSamplerHandler, 0);

    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, mBlurTexture);
    glUniform1i(mBlurTextureSamplerHandler, 1);

    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, mHighPassBlurTexture);
    glUniform1i(mHighPassBlurTextureSamplerHandler, 2);

    glDrawArrays(GL_TRIANGLES, 0, BaseFilter::DEFAULT_VERTEX_COUNT);

    glDisableVertexAttribArray(mVertexPosHandler);
    glDisableVertexAttribArray(mTextureCoordinateHandler);
    glBindTexture(GL_TEXTURE_2D, 0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    return inputTextureId;
}

void SkinBuffFilter::setBlurTexture(GLuint blurTexture) {
    mBlurTexture = blurTexture;
}

void SkinBuffFilter::setHighPassBlurTexture(GLuint highPassBlurTexture) {
    mHighPassBlurTexture = highPassBlurTexture;
}
