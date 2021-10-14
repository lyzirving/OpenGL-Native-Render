//
// Created by liuyuzhou on 2021/10/14.
//
#include "ColorAdjustFilter.h"
#include "Common.h"
#include "GlUtil.h"
#include "LogUtil.h"

#define TAG "ColorAdjustFilter"

void ColorAdjustFilter::adjustProperty(const char *property, int progress) {
    if (progress < 0) {
        progress = 0;
    } else if (progress > 100) {
        progress = 100;
    }
    if (property == nullptr || std::strlen(property) == 0) {
        LogUtil::logI(TAG, {"adjustProperty: filter prop is invalid"});
    } else if (std::strcmp(render::FILTER_PROP_SHARPEN, property) == 0) {
        if (progress < 50) {
            mSharpness = ((float)(50 - progress)) / 50 * -4;
        } else {
            mSharpness = ((float)(progress - 50)) / 50 * 4;
        }
        LogUtil::logI(TAG, {"adjustProperty: sharpen, progress = ", std::to_string(progress), ", sharpness = ", std::to_string(mSharpness)});
    } else if (std::strcmp(render::FILTER_PROP_CONTRAST, property) == 0) {
        if (progress >= 50) {
            mContrast = ((float) (progress - 50)) / ((float) 50) + 1;
        } else {
            mContrast = (float) progress / (float) (50);
        }
        LogUtil::logI(TAG, {"adjustProperty: contrast, progress = ", std::to_string(progress), ", contrast = ", std::to_string(mContrast)});
    } else if (std::strcmp(render::FILTER_PROP_SATURATION, property) == 0) {
        if (progress < 50) {
            mSaturation = 1 - ((float)(50 - progress)) / 50;
        } else {
            mSaturation = 1 + ((float)(progress - 50)) / 50;
        }
        LogUtil::logI(TAG, {"adjustProperty: saturation, progress = ", std::to_string(progress), ", saturation = ", std::to_string(mSaturation)});
    }
}

void ColorAdjustFilter::loadShader() {
    auto gUtil = GlUtil::self();
    if (mVertexShader == nullptr) { mVertexShader = gUtil->readAssets("shader/color_adj_vertex_shader.glsl"); }
    if (mTextureShader == nullptr) { mTextureShader = gUtil->readAssets("shader/color_adj_fragment_shader.glsl"); }
}

void ColorAdjustFilter::initHandler() {
    BaseFilter::initHandler();
    mImgWidthHandler = glGetUniformLocation(mProgram, "uImgWidthFactor");
    mImgHeightHandler = glGetUniformLocation(mProgram, "uImgHeightFactor");
    mSharpnessHandler = glGetUniformLocation(mProgram, "uSharpness");
    mContrastHandler = glGetUniformLocation(mProgram, "uContrast");
    mSaturationHandler = glGetUniformLocation(mProgram, "uSaturation");
}

GLint ColorAdjustFilter::onDraw(GLint inputTextureId) {
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

    glUniform1f(mImgWidthHandler, ((float)1) / mWidth);
    glUniform1f(mImgHeightHandler, ((float)1) / mHeight);
    glUniform1f(mSharpnessHandler, mSharpness);
    glUniform1f(mContrastHandler, mContrast);
    glUniform1f(mSaturationHandler, mSaturation);

    glDrawArrays(GL_TRIANGLES, 0, BaseFilter::DEFAULT_VERTEX_COUNT);

    glDisableVertexAttribArray(mVertexPosHandler);
    glDisableVertexAttribArray(mTextureCoordinateHandler);
    glBindTexture(GL_TEXTURE_2D, 0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    return inputTextureId;
}
