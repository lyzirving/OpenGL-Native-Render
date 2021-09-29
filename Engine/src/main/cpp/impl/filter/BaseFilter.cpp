//
// Created by liuyuzhou on 2021/9/2.
//
#include "BaseFilter.h"
#include "GlUtil.h"
#include "LogUtil.h"

#define TAG "BaseFilter"

BaseFilter::BaseFilter() = default;

BaseFilter::~BaseFilter() {
    delete[] mFilterType;
}

void BaseFilter::adjust(int progress) {}

void BaseFilter::destroy() {
    if (mVertexShader != nullptr) {
        delete mVertexShader;
        mVertexShader = nullptr;
    }
    if (mTextureShader != nullptr) {
        delete mTextureShader;
        mTextureShader = nullptr;
    }
    if (mVertex != nullptr) {
        delete[] mVertex;
        mVertex = nullptr;
    }
    if (mTextureCoordinate != nullptr) {
        delete[] mTextureCoordinate;
        mTextureCoordinate = nullptr;
    }
    if (mTextureId != 0) {
        glDeleteTextures(1, &mTextureId);
        mTextureId = 0;
    }
    if (mProgram != 0) {
        glDeleteProgram(mProgram);
        mProgram = 0;
    }
    mInitialized = false;
}

const char * BaseFilter::getType() {
    return mFilterType;
}

GLuint BaseFilter::getFrameBuffer() { return 0; }

void BaseFilter::init() {
    if (!mInitialized) {
        loadShader();
        auto pUtil = GlUtil::self();
        if (!(mProgram = pUtil->loadProgram(mVertexShader, mTextureShader))) {
            LogUtil::logI(TAG, {"init: failed to create program"});
            return;
        }
        preInit();
        initHandler();
        initBuffer();
        initFrameBuffer();
        initTexture();
        postInit();
        mInitialized = true;
    }
}

void BaseFilter::initHandler() {
    mVertexPosHandler = glGetAttribLocation(mProgram, "aVertexPos");
    mTextureCoordinateHandler = glGetAttribLocation(mProgram, "aTextureCoordinate");
    mTextureSamplerHandler = glGetUniformLocation(mProgram, "sTextureSampler");
}

void BaseFilter::initBuffer() {
    if (mVertex == nullptr) {
        mVertex = new GLfloat[DEFAULT_VERTEX_COUNT * 3]{
                //right - top
                1, 1, 0,
                //left - bottom
                -1, -1, 0,
                //left - top
                -1, 1, 0,
                //right - top
                1, 1, 0,
                //right - bottom
                1, -1, 0,
                //left - bottom
                -1, -1, 0
        };
    }
    if (mTextureCoordinate == nullptr) {
        mTextureCoordinate = new GLfloat[DEFAULT_VERTEX_COUNT * 2]{
                //right - top
                1, 0,
                //left - bottom
                0, 1,
                //left - top
                0, 0,
                //right - top
                1, 0,
                //right - bottom
                1, 1,
                //left - bottom
                0, 1,
        };
    }
    if (mBufferId[0] != 0 || mBufferId[1] != 0) { glDeleteBuffers(2, mBufferId); }
    //generate buffer for vertex and texture coordinate
    glGenBuffers(2, mBufferId);

    glBindBuffer(GL_ARRAY_BUFFER, mBufferId[0]);
    glBufferData(GL_ARRAY_BUFFER, DEFAULT_VERTEX_COUNT * 3 * sizeof(GLfloat), mVertex,
                 GL_STATIC_DRAW);
    glEnableVertexAttribArray(mVertexPosHandler);

    glBindBuffer(GL_ARRAY_BUFFER, mBufferId[1]);
    glBufferData(GL_ARRAY_BUFFER, DEFAULT_VERTEX_COUNT * 2 * sizeof(GLfloat), mTextureCoordinate,
                 GL_STATIC_DRAW);
    glEnableVertexAttribArray(mTextureCoordinateHandler);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void BaseFilter::initTexture() {}

void BaseFilter::initFrameBuffer() {}

bool BaseFilter::initialized() {
    return mInitialized;
}

void BaseFilter::loadShader() {
    auto gUtil = GlUtil::self();
    if (mVertexShader == nullptr) { mVertexShader = gUtil->readAssets("shader/default_vertex_shader.glsl"); }
    if (mTextureShader == nullptr) { mTextureShader = gUtil->readAssets("shader/default_fragment_shader.glsl"); }
}

GLint BaseFilter::onDraw(GLint inputTextureId) {
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

    glDrawArrays(GL_TRIANGLES, 0, BaseFilter::DEFAULT_VERTEX_COUNT);

    glDisableVertexAttribArray(mVertexPosHandler);
    glDisableVertexAttribArray(mTextureCoordinateHandler);
    glBindTexture(GL_TEXTURE_2D, 0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    return inputTextureId;
}

GLint BaseFilter::onDraw(GLint inputFrameBufferId, GLint inputTextureId) {}

void BaseFilter::onPause() {
    mInitialized = false;
    if (mProgram != 0) {
        glDeleteProgram(mProgram);
        mProgram = 0;
    }
    if (mTextureId != 0) {
        glDeleteTextures(1, &mTextureId);
        mTextureId = 0;
    }
    if (mBufferId[0] != 0 || mBufferId[1] != 0) {
        glDeleteBuffers(2, mBufferId);
        mBufferId[0] = 0;
        mBufferId[1] = 0;
    }
}

void BaseFilter::onResume() {}

void BaseFilter::preInit() {}

void BaseFilter::postInit() {}

void BaseFilter::setOutputSize(GLint width, GLint height) {
    if (width != mWidth || height != mHeight) { mInitialized = false; }
    mWidth = width;
    mHeight = height;
}

