//
// Created by liuyuzhou on 2021/9/9.
//
#include "ContrastFilter.h"
#include "GlUtil.h"
#include "LogUtil.h"

#define TAG "ContrastFilter"

void ContrastFilter::adjust(int progress) {
    if (progress < 0) {
        progress = 0;
    } else if (progress > 100) {
        progress = 100;
    }
    if (progress >= 50) {
        mContrast = ((float) (progress - 50)) / ((float) 50) + 1;
    } else {
        mContrast = (float) progress / (float) (50);
    }
    LogUtil::logI(TAG, {"adjust: progress = ", std::to_string(progress), ", contrast = ", std::to_string(mContrast)});
}

void ContrastFilter::init() {
    if (!mInitialized) { LogUtil::logI(TAG, {"init: begin to initialized filter"}); }
    BaseFilter::init();
}

void ContrastFilter::loadShader() {
    auto gUtil = GlUtil::self();
    if (mVertexShader == nullptr) { mVertexShader = gUtil->readAssets("shader/default_vertex_shader.glsl"); }
    if (mTextureShader == nullptr) { mTextureShader = gUtil->readAssets("shader/contrast_shader.glsl"); }
}

void ContrastFilter::initHandler() {
    BaseFilter::initHandler();
    mContrastHandler = glGetUniformLocation(mProgram, "uContrast");
}

GLint ContrastFilter::onDraw(GLint inputTextureId) {
    glUseProgram(mProgram);

    glBindBuffer(GL_ARRAY_BUFFER, mBufferId[0]);
    glVertexAttribPointer(mVertexPosHandler, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), nullptr);
    glEnableVertexAttribArray(mVertexPosHandler);

    glBindBuffer(GL_ARRAY_BUFFER, mBufferId[1]);
    glVertexAttribPointer(mTextureCoordinateHandler, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(GLfloat),nullptr);
    glEnableVertexAttribArray(mTextureCoordinateHandler);

    glUniform1f(mContrastHandler, mContrast);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, inputTextureId);
    glUniform1i(mTextureSamplerHandler, 0);

    glDrawArrays(GL_TRIANGLES, 0, BaseFilter::DEFAULT_VERTEX_COUNT);

    glDisableVertexAttribArray(mVertexPosHandler);
    glDisableVertexAttribArray(mTextureCoordinateHandler);
    glBindTexture(GL_TEXTURE_2D, 0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    return inputTextureId;
}

