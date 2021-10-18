//
// Created by liuyuzhou on 2021/10/15.
//
#include "BeautifyFaceFilter.h"
#include "LogUtil.h"
#include "Common.h"

#define TAG "BeautifyFaceFilter"
#define FRAME_BUFFER_COUNT 5
#define INDEX_DRAW 4

BeautifyFaceFilter::BeautifyFaceFilter() {
    mSkinFilter = new SkinFilter;
    mGaussianFilter = new GaussianFilter;
    mHighPassFilter = new HighPassFilter;
    mSkinBuffFilter = new SkinBuffFilter;
    mFilterType = new char[strlen(render::FILTER_TYPE_GROUP) + 1];
    std::memcpy(mFilterType, render::FILTER_TYPE_GROUP, strlen(render::FILTER_TYPE_GROUP));
    mFilterType[strlen(render::FILTER_TYPE_GROUP)] = 0;
}

BeautifyFaceFilter::~BeautifyFaceFilter() {
    delete mSkinFilter;
    delete mGaussianFilter;
    delete mHighPassFilter;
    delete mSkinBuffFilter;
    mSkinFilter = nullptr;
    mGaussianFilter = nullptr;
    mHighPassFilter = nullptr;
    mSkinBuffFilter = nullptr;
}

void BeautifyFaceFilter::adjustProperty(const char *property, int progress) {
    if (property == nullptr || std::strlen(property) == 0) {
        LogUtil::logI(TAG, {"adjustProperty: filter prop is invalid"});
    } else if (std::strcmp(render::FILTER_PROP_SKIN_BUFF, property) == 0) {
        mSkinBuffFilter->adjustProperty(property, progress);
    } else if (std::strcmp(render::FILTER_PROP_BEAUTIFY_SKIN, property) == 0) {
        mSkinFilter->adjustProperty(property, progress);
    } else {
        LogUtil::logI(TAG, {"adjustProperty: do not handle property ", property});
    }
}

void BeautifyFaceFilter::destroy() {
    BaseFilter::destroy();
    mSkinFilter->destroy();
    mGaussianFilter->destroy();
    mHighPassFilter->destroy();
    mSkinBuffFilter->destroy();
    if (mFrameBuffers != nullptr) {
        glDeleteFramebuffers(FRAME_BUFFER_COUNT, mFrameBuffers);
        delete[] mFrameBuffers;
    }
    mFrameBuffers = nullptr;
    if (mTextures != nullptr) {
        glDeleteTextures(GL_TEXTURE_2D, mTextures);
        delete[] mTextures;
    }
    mTextures = nullptr;
}

void BeautifyFaceFilter::init() {
    if (!mInitialized) {
        preInit();
        mSkinFilter->init();
        mGaussianFilter->init();
        mHighPassFilter->init();
        mSkinBuffFilter->init();
        initFrameBuffer();
        initTexture();
        postInit();
        mInitialized = true;
    }
}

void BeautifyFaceFilter::initFrameBuffer() {
    if (mFrameBuffers != nullptr) {
        glDeleteFramebuffers(FRAME_BUFFER_COUNT, mFrameBuffers);
        delete[] mFrameBuffers;
    }
    mFrameBuffers = new GLuint[FRAME_BUFFER_COUNT];
    glGenFramebuffers(FRAME_BUFFER_COUNT, mFrameBuffers);
}

void BeautifyFaceFilter::initTexture() {
    if (mTextures != nullptr) {
        glDeleteTextures(GL_TEXTURE_2D, mTextures);
        delete[] mTextures;
    }
    mTextures = new GLuint[FRAME_BUFFER_COUNT];
    glGenTextures(FRAME_BUFFER_COUNT, mTextures);
    for (int i = 0; i < FRAME_BUFFER_COUNT; ++i) {
        glBindTexture(GL_TEXTURE_2D, mTextures[i]);
        glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, mWidth, mHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE,nullptr);

        glBindFramebuffer(GL_FRAMEBUFFER, mFrameBuffers[i]);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, mTextures[i], 0);

        glBindTexture(GL_TEXTURE_2D, 0);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }
}

GLint BeautifyFaceFilter::onDraw(GLint inputTextureId) {
    glBindFramebuffer(GL_FRAMEBUFFER, mFrameBuffers[0]);
    mSkinFilter->onDraw(inputTextureId);
    GLint skinTexture = mTextures[0];

    glBindFramebuffer(GL_FRAMEBUFFER, mFrameBuffers[1]);
    mGaussianFilter->setBlurSize(1, 1);
    mGaussianFilter->onDraw(mFrameBuffers[1], skinTexture);
    GLint blurTexture = mTextures[1];

    glBindFramebuffer(GL_FRAMEBUFFER, mFrameBuffers[2]);
    mHighPassFilter->setBlurTexture(blurTexture);
    mHighPassFilter->onDraw(skinTexture);
    GLint highPassTexture = mTextures[2];

    glBindFramebuffer(GL_FRAMEBUFFER, mFrameBuffers[3]);
    mGaussianFilter->onDraw(mFrameBuffers[3], highPassTexture);
    GLint highPassBlurTexture = mTextures[3];

    glBindFramebuffer(GL_FRAMEBUFFER, mFrameBuffers[INDEX_DRAW]);
    mSkinBuffFilter->setBlurTexture(blurTexture);
    mSkinBuffFilter->setHighPassBlurTexture(highPassBlurTexture);
    mSkinBuffFilter->onDraw(skinTexture);

    return mTextures[INDEX_DRAW];
}

void BeautifyFaceFilter::onPause() {
    BaseFilter::onPause();
    mSkinFilter->onPause();
    mGaussianFilter->onPause();
    mHighPassFilter->onPause();
    mSkinBuffFilter->onPause();
    if (mFrameBuffers != nullptr) {
        glDeleteFramebuffers(FRAME_BUFFER_COUNT, mFrameBuffers);
        delete[] mFrameBuffers;
    }
    mFrameBuffers = nullptr;
    if (mTextures != nullptr) {
        glDeleteTextures(GL_TEXTURE_2D, mTextures);
        delete[] mTextures;
    }
    mTextures = nullptr;
}


void BeautifyFaceFilter::setOutputSize(GLint width, GLint height) {
    BaseFilter::setOutputSize(width, height);
    mSkinFilter->setOutputSize(width, height);
    mGaussianFilter->setOutputSize(width, height);
    mHighPassFilter->setOutputSize(width, height);
    mSkinBuffFilter->setOutputSize(width, height);
}

void BeautifyFaceFilter::onResume() {
    BaseFilter::onResume();
    init();
}

