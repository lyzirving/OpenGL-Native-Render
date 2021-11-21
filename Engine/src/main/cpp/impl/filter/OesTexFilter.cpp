//
// Created by lyzirving on 2021/11/21.
//
#include "OesTexFilter.h"
#include "GlUtil.h"
#include "Log.h"
#include "MatrixUtil.h"

/**
 * this file must be included after <GLES2/gl2.h>
 * BaseFilter.h contains file <GLES2/gl2.h>
 */
#include <GLES2/gl2ext.h>

#define TAG "OesTexFilter"

OesTexFilter::OesTexFilter() {
    MatrixUtil::setIdentityM(mTexMatrix, 0);
}

OesTexFilter::~OesTexFilter() = default;

void OesTexFilter::init() {
    if (!mInitialized) { LOG_I("%s_init: begin to init", TAG); }
    BaseFilter::init();
}

void OesTexFilter::initHandler() {
    mVertexPosHandler = glGetAttribLocation(mProgram, "aVertexPos");
    mTextureCoordinateHandler = glGetAttribLocation(mProgram, "aTextureCoordinate");
    mOesTexMatrixHandler = glGetUniformLocation(mProgram, "uOesTexMatrix");
    mOesTextureSamplerHandler = glGetUniformLocation(mProgram, "sOesTextureSampler");
}

void OesTexFilter::loadShader() {
    auto gUtil = GlUtil::self();
    if (mVertexShader == nullptr) {
        mVertexShader = gUtil->readAssets("shader/oes_vertex_shader.glsl");
    }
    if (mTextureShader == nullptr) {
        mTextureShader = gUtil->readAssets("shader/oes_fragment_shader.glsl");
    }
}

GLint OesTexFilter::onDraw(GLint inputTextureId) {
    glUseProgram(mProgram);

    glBindBuffer(GL_ARRAY_BUFFER, mBufferId[0]);
    glVertexAttribPointer(mVertexPosHandler, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), nullptr);
    glEnableVertexAttribArray(mVertexPosHandler);
    glBindBuffer(GL_ARRAY_BUFFER, mBufferId[1]);
    glVertexAttribPointer(mTextureCoordinateHandler, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(GLfloat),nullptr);
    glEnableVertexAttribArray(mTextureCoordinateHandler);

    glUniformMatrix4fv(mOesTexMatrixHandler, 1, false, mTexMatrix);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_EXTERNAL_OES, inputTextureId);
    glUniform1i(mOesTextureSamplerHandler, 0);

    glDrawArrays(GL_TRIANGLES, 0, BaseFilter::DEFAULT_VERTEX_COUNT);

    glDisableVertexAttribArray(mVertexPosHandler);
    glDisableVertexAttribArray(mTextureCoordinateHandler);
    glBindTexture(GL_TEXTURE_2D, 0);
    MatrixUtil::setIdentityM(mTexMatrix, 0);

    return inputTextureId;
}

void OesTexFilter::setTexMatrix(const GLfloat* matrix) {
    for (int i = 0; i < 16; ++i) { mTexMatrix[i] = matrix[i]; }
}
