//
// Created by liuyuzhou on 2021/10/15.
//
#include "SkinFilter.h"
#include "Common.h"
#include "GlUtil.h"
#include "LogUtil.h"

#define TAG "SkinFilter"


void SkinFilter::adjustProperty(const char *property, int progress) {
    if (property == nullptr || std::strlen(property) == 0) {
        LogUtil::logI(TAG, {"adjustProperty: filter prop is invalid"});
    } else if (std::strcmp(render::FILTER_PROP_BEAUTIFY_SKIN, property) == 0) {
        mAlpha = (float)progress / 100;
        LogUtil::logI(TAG, {"adjustProperty: beautify skin, intensity = ", std::to_string(mAlpha), ", progress = ", std::to_string(progress)});
    } else {
        LogUtil::logI(TAG, {"adjustProperty: do not handle property ", property});
    }
}

void SkinFilter::destroy() {
    BaseFilter::destroy();
    if (mGrayTexture != 0) {
        glDeleteTextures(1, &mGrayTexture);
        mGrayTexture = 0;
    }
    if (mLookupTexture != 0) {
        glDeleteTextures(1, &mLookupTexture);
        mLookupTexture = 0;
    }
}

void SkinFilter::initHandler() {
    BaseFilter::initHandler();
    mGrayTextureSampler = glGetUniformLocation(mProgram, "sGrayTextureSampler");
    mLookupTextureSampler = glGetUniformLocation(mProgram, "sLookupTextureSampler");

    mLevelRangeHandler = glGetUniformLocation(mProgram, "uLevelRangeInv");
    mLevelBlackHandler = glGetUniformLocation(mProgram, "uLevelBlack");
    mAlphaHandler = glGetUniformLocation(mProgram, "uAlpha");
}

void SkinFilter::initTexture() {
    if (mGrayTexture != 0) { glDeleteTextures(1, &mGrayTexture); }
    if (mLookupTexture != 0) { glDeleteTextures(1, &mLookupTexture); }
    GlUtil* glUtil = GlUtil::self();
    mGrayTexture = glUtil->generateTextureFromAssets("texture/skin_gray.png");
    mLookupTexture = glUtil->generateTextureFromAssets("texture/skin_lookup.png");
    if (mGrayTexture == 0) { LogUtil::logI(TAG, {"initTexture: failed to generate skin gray"}); }
    if (mLookupTexture == 0) { LogUtil::logI(TAG, {"initTexture: failed to generate look up"}); }
}

void SkinFilter::loadShader() {
    auto gUtil = GlUtil::self();
    if (mVertexShader == nullptr) { mVertexShader = gUtil->readAssets("shader/default_vertex_shader.glsl"); }
    if (mTextureShader == nullptr) { mTextureShader = gUtil->readAssets("shader/skin_fragment_shader.glsl"); }
}

GLint SkinFilter::onDraw(GLint inputTextureId) {
    glUseProgram(mProgram);

    glBindBuffer(GL_ARRAY_BUFFER, mBufferId[0]);
    glVertexAttribPointer(mVertexPosHandler, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), nullptr);
    glEnableVertexAttribArray(mVertexPosHandler);

    glBindBuffer(GL_ARRAY_BUFFER, mBufferId[1]);
    glVertexAttribPointer(mTextureCoordinateHandler, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(GLfloat),nullptr);
    glEnableVertexAttribArray(mTextureCoordinateHandler);

    glUniform1f(mLevelRangeHandler, mLevelRangeIne);
    glUniform1f(mLevelBlackHandler, mLevelBlack);
    glUniform1f(mAlphaHandler, mAlpha);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, inputTextureId);
    glUniform1i(mTextureSamplerHandler, 0);

    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, mGrayTexture);
    glUniform1i(mGrayTextureSampler, 1);

    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, mLookupTexture);
    glUniform1i(mLookupTextureSampler, 2);

    glDrawArrays(GL_TRIANGLES, 0, BaseFilter::DEFAULT_VERTEX_COUNT);

    glDisableVertexAttribArray(mVertexPosHandler);
    glDisableVertexAttribArray(mTextureCoordinateHandler);
    glBindTexture(GL_TEXTURE_2D, 0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    return inputTextureId;
}

void SkinFilter::onPause() {
    BaseFilter::onPause();
    if (mGrayTexture != 0) {
        glDeleteTextures(1, &mGrayTexture);
        mGrayTexture = 0;
    }
    if (mLookupTexture != 0) {
        glDeleteTextures(1, &mLookupTexture);
        mLookupTexture = 0;
    }
}
