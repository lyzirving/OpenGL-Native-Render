//
// Created by liuyuzhou on 2021/9/8.
//
#include <android/bitmap.h>

#include "BackgroundFilter.h"
#include "GlUtil.h"
#include "LogUtil.h"

#define TAG "BackgroundFilter"

void BackgroundFilter::destroy() {
    if (mFrameBufferTextureId != 0) {
        glDeleteTextures(1, &mFrameBufferTextureId);
        mFrameBufferTextureId = 0;
    }
    if (mFrameBufferId != 0) {
        glDeleteFramebuffers(1, &mFrameBufferId);
        mFrameBufferId = 0;
    }
    if (mPixel != nullptr) {
        delete[] mPixel;
        mPixel = nullptr;
    }
    BaseFilter::destroy();
}

int BackgroundFilter::getBitmapWidth() {
    return mBitmapWidth;
}

int BackgroundFilter::getBitmapHeight() {
    return mBitmapHeight;
}

void BackgroundFilter::init() {
    BaseFilter::init();
    LogUtil::logI(TAG, {"init: res = ", std::to_string(mInitialized)});
}

void BackgroundFilter::initBuffer() {
    if (mVertex != nullptr) { delete[] mVertex; }
    float horRatio = ((float)mBitmapWidth) / ((float)mWidth);
    float verRatio = ((float)mBitmapHeight) / ((float)mHeight);
    mVertex = new GLfloat[DEFAULT_VERTEX_COUNT * 3]{
            //right - top
            horRatio, verRatio, 0,
            //left - bottom
            -horRatio, -verRatio, 0,
            //left - top
            -horRatio, verRatio, 0,
            //right - top
            horRatio, verRatio, 0,
            //right - bottom
            horRatio, -verRatio, 0,
            //left - bottom
            -horRatio, -verRatio, 0
    };
    LogUtil::logI(TAG, {"initBuffer: hor = ", std::to_string(horRatio), ", ver = ", std::to_string(verRatio)});
    BaseFilter::initBuffer();
}

void BackgroundFilter::initFrameBuffer() {
    if (mFrameBufferId != 0) { glDeleteFramebuffers(1, &mFrameBufferId); }
    if (mFrameBufferTextureId != 0) { glDeleteTextures(1, &mFrameBufferTextureId); }
    glGenFramebuffers(1, &mFrameBufferId);

    glGenTextures(1, &mFrameBufferTextureId);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glBindTexture(GL_TEXTURE_2D, mFrameBufferTextureId);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, mWidth, mHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);

    glBindFramebuffer(GL_FRAMEBUFFER, mFrameBufferId);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, mFrameBufferTextureId, 0);
    glBindTexture(GL_TEXTURE_2D, 0);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void BackgroundFilter::initTexture() {
    if (mPixel == nullptr) {
        LogUtil::logI(TAG, {"initTexture: no pixel data yet"});
    }
    if (mTextureId != 0) { glDeleteTextures(1, &mTextureId); }
    glGenTextures(1, &mTextureId);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glBindTexture(GL_TEXTURE_2D, mTextureId);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, mBitmapWidth, mBitmapHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, mPixel);
}

GLint BackgroundFilter::onDraw(GLint inputTextureId) {
    glBindFramebuffer(GL_FRAMEBUFFER, mFrameBufferId);

    glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
    glUseProgram(mProgram);

    glBindBuffer(GL_ARRAY_BUFFER, mBufferId[0]);
    glVertexAttribPointer(mVertexPosHandler, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), nullptr);
    glEnableVertexAttribArray(mVertexPosHandler);

    glBindBuffer(GL_ARRAY_BUFFER, mBufferId[1]);
    glVertexAttribPointer(mTextureCoordinateHandler, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(GLfloat),nullptr);
    glEnableVertexAttribArray(mTextureCoordinateHandler);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, mTextureId);
    glUniform1i(mTextureSamplerHandler, 0);

    glDrawArrays(GL_TRIANGLES, 0, BaseFilter::DEFAULT_VERTEX_COUNT);

    glDisableVertexAttribArray(mVertexPosHandler);
    glDisableVertexAttribArray(mTextureCoordinateHandler);
    glBindTexture(GL_TEXTURE_2D, 0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    return mFrameBufferTextureId;
}

void BackgroundFilter::onPause() {
    BaseFilter::onPause();
    if (mFrameBufferId != 0) {
        glDeleteFramebuffers(1, &mFrameBufferId);
        mFrameBufferId = 0;
    }
    if (mFrameBufferTextureId != 0) {
        glDeleteTextures(1, &mFrameBufferTextureId);
        mFrameBufferTextureId = 0;
    }
    if (mPixel != nullptr) {
        delete[] mPixel;
        mPixel = nullptr;
    }
}

void BackgroundFilter::setBitmap(JNIEnv* env, jobject bitmap) {
    AndroidBitmapInfo info;
    if (AndroidBitmap_getInfo(env, bitmap, &info) < 0) {
        LogUtil::logI(TAG, {"setBitmap: failed to get bitmap info"});
        return;
    }
    if (info.format != ANDROID_BITMAP_FORMAT_RGBA_8888) {
        LogUtil::logI(TAG, {"setBitmap: format is not RGBA 8888"});
        return;
    }
    mBitmapWidth = info.width;
    mBitmapHeight = info.height;
    unsigned char* pixel = nullptr;
    if (AndroidBitmap_lockPixels(env, bitmap, reinterpret_cast<void **>(&pixel)) < 0) {
        LogUtil::logI(TAG, {"setBitmap: failed to get pixel"});
        return;
    }
    if (mPixel != nullptr) { delete[] mPixel; }
    mPixel = new unsigned char[mBitmapWidth * mBitmapHeight * 4];
    for (int i = 0; i < mBitmapWidth * mBitmapHeight * 4; ++i) {
        mPixel[i] = pixel[i];
    }
    AndroidBitmap_unlockPixels(env, bitmap);
    LogUtil::logI(TAG, {"setBitmap: succeed: bitmap width = ", {std::to_string(mBitmapWidth)}, {", bitmap height = "}, {std::to_string(mBitmapHeight)}});
}

