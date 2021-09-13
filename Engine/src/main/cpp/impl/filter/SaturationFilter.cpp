//
// Created by liuyuzhou on 2021/9/13.
//
#include <LogUtil.h>
#include "SaturationFilter.h"

#define TAG "SaturationFilter"

void SaturationFilter::adjust(int progress) {
    if (progress < 0) {
        progress = 0;
    } else if (progress > 100) {
        progress = 100;
    }
    if (progress < 50) {
        mSaturation = 1 - ((float)(50 - progress)) / 50;
    } else {
        mSaturation = 1 + ((float)(progress - 50)) / 50;
    }
    LogUtil::logI(TAG, {"adjust: progress = ", std::to_string(progress), ", saturation = ", std::to_string(mSaturation)});
}

void SaturationFilter::initHandler() {
    BaseFilter::initHandler();
    mSaturationHandler = glGetUniformLocation(mProgram, "uSaturation");
}

void SaturationFilter::loadShader() {
    auto gUtil = GlUtil::self();
    if (mVertexShader == nullptr) { mVertexShader = gUtil->readAssets("shader/default_vertex_shader.glsl"); }
    if (mTextureShader == nullptr) { mTextureShader = gUtil->readAssets("shader/saturation_fragment_shader.glsl"); }
}

GLint SaturationFilter::onDraw(GLint inputTextureId) {
    LogUtil::logI(TAG, {"onDraw: ", std::to_string(mSaturation)});
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

    glUniform1f(mSaturationHandler, mSaturation);

    glDrawArrays(GL_TRIANGLES, 0, BaseFilter::DEFAULT_VERTEX_COUNT);

    glDisableVertexAttribArray(mVertexPosHandler);
    glDisableVertexAttribArray(mTextureCoordinateHandler);
    glBindTexture(GL_TEXTURE_2D, 0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    return inputTextureId;
}

