//
// Created by liuyuzhou on 2021/10/22.
//
#include "ModelFilter.h"
#include "GlUtil.h"

void ModelFilter::initBuffer() {
    BaseFilter::initBuffer();
}

void ModelFilter::initHandler() {
    BaseFilter::initHandler();
    mVertexNormalHandler = glGetAttribLocation(mProgram, "aVertexNormal");
    mMVPMatrixHandler = glGetUniformLocation(mProgram, "uMVPMatrix");
    mMMatrixHandler = glGetUniformLocation(mProgram, "uMMatrix");
    mLightLocationHandler = glGetUniformLocation(mProgram, "uLightLocation");
    mAmbientHandler = glGetUniformLocation(mProgram, "aAmbient");
    mDiffuseHandler = glGetUniformLocation(mProgram, "uDiffuse");
}

void ModelFilter::loadShader() {
    auto gUtil = GlUtil::self();
    if (mVertexShader == nullptr) { mVertexShader = gUtil->readAssets("shader/model_vertex_shader.glsl"); }
    if (mTextureShader == nullptr) { mTextureShader = gUtil->readAssets("shader/model_fragment_shader.glsl"); }
}
