//
// Created by liuyuzhou on 2021/9/14.
//
#include "ExposureFilter.h"
#include "LogUtil.h"

#define TAG "ExposureFilter"

void ExposureFilter::adjust(int progress) {
    if(progress < 0) {
        progress = 0;
    } else if (progress > 100) {
        progress = 100;
    }
    mExposure = ((float)(progress)) / 100 * 2;
    LogUtil::logI(TAG, {"adjust: progress = ", std::to_string(progress), ", exposure = ", std::to_string(mExposure)});
}

void ExposureFilter::initHandler() {
    BaseFilter::initHandler();
    mExposureHandler = glGetUniformLocation(mProgram, "uExposure");
}

void ExposureFilter::loadShader() {
    auto gUtil = GlUtil::self();
    if (mVertexShader == nullptr) { mVertexShader = gUtil->readAssets("shader/default_vertex_shader.glsl"); }
    if (mTextureShader == nullptr) { mTextureShader = gUtil->readAssets("shader/exposure_fragment_shader.glsl"); }
}

GLint ExposureFilter::onDraw(GLint inputTextureId) {
    glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
    glUseProgram(mProgram);

    glBindBuffer(GL_ARRAY_BUFFER, mBufferId[0]);
    glVertexAttribPointer(mVertexPosHandler, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), nullptr);
    glEnableVertexAttribArray(mVertexPosHandler);

    glBindBuffer(GL_ARRAY_BUFFER, mBufferId[1]);
    glVertexAttribPointer(mTextureCoordinateHandler, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(GLfloat),nullptr);
    glEnableVertexAttribArray(mTextureCoordinateHandler);

    glUniform1f(mExposureHandler, mExposure);

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

