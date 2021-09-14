//
// Created by liuyuzhou on 2021/9/14.
//
#include "HighlightShadowFilter.h"
#include "LogUtil.h"

#define TAG "HighlightShadowFilter"

void HighlightShadowFilter::adjustHighlight(int progress) {
    if (progress < 0) {
        progress = 0;
    } else if (progress > 100) {
        progress = 100;
    }
    mHighlight = ((float)(100 - progress)) / 100;
    LogUtil::logI(TAG, {"adjustHighlight: progress = ", std::to_string(progress), ", highlight = ", std::to_string(mHighlight)});
}

void HighlightShadowFilter::adjustShadow(int progress) {
    if (progress < 0) {
        progress = 0;
    } else if (progress > 100) {
        progress = 100;
    }
    mShadow = ((float)progress) / 100;
    LogUtil::logI(TAG, {"adjustShadow: progress = ", std::to_string(progress), ", shadow = ", std::to_string(mShadow)});
}

void HighlightShadowFilter::initHandler() {
    BaseFilter::initHandler();
    mShadowHandler = glGetUniformLocation(mProgram, "uShadows");
    mHighlightHandler = glGetUniformLocation(mProgram, "uHighlights");
}

void HighlightShadowFilter::loadShader() {
    auto gUtil = GlUtil::self();
    if (mVertexShader == nullptr) { mVertexShader = gUtil->readAssets("shader/default_vertex_shader.glsl"); }
    if (mTextureShader == nullptr) { mTextureShader = gUtil->readAssets("shader/highlight_shadow_fragment_shader.glsl"); }
}

GLint HighlightShadowFilter::onDraw(GLint inputTextureId) {
    glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
    glUseProgram(mProgram);

    glBindBuffer(GL_ARRAY_BUFFER, mBufferId[0]);
    glVertexAttribPointer(mVertexPosHandler, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), nullptr);
    glEnableVertexAttribArray(mVertexPosHandler);

    glBindBuffer(GL_ARRAY_BUFFER, mBufferId[1]);
    glVertexAttribPointer(mTextureCoordinateHandler, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(GLfloat),nullptr);
    glEnableVertexAttribArray(mTextureCoordinateHandler);

    glUniform1f(mHighlightHandler, mHighlight);
    glUniform1f(mShadowHandler, mShadow);

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

