//
// Created by liuyuzhou on 2021/9/14.
//
#include "HighlightShadowFilter.h"
#include "Common.h"
#include "GlUtil.h"
#include "LogUtil.h"

#define TAG "HighlightShadowFilter"

void HighlightShadowFilter::adjustProperty(const char *property, int progress) {
    if (progress < 0) {
        progress = 0;
    } else if (progress > 100) {
        progress = 100;
    }
    if (property == nullptr || std::strlen(property) == 0) {
        LogUtil::logI(TAG, {"adjustProperty: filter prop is invalid"});
    } else if (std::strcmp(render::FILTER_PROP_HIGHLIGHT, property) == 0) {
        mHighlight = ((float)(100 - progress)) / 100;
        LogUtil::logI(TAG, {"adjustProperty: highlight, progress = ", std::to_string(progress), ", highlight = ", std::to_string(mHighlight)});
    } else if (std::strcmp(render::FILTER_PROP_SHADOW, property) == 0) {
        mShadow = ((float)progress) / 100;
        LogUtil::logI(TAG, {"adjustProperty: shadow, progress = ", std::to_string(progress), ", shadow = ", std::to_string(mShadow)});
    }
}

void HighlightShadowFilter::init() {
    if (!mInitialized) { LogUtil::logI(TAG, {"init: begin to initialized filter"}); }
    BaseFilter::init();
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

