//
// Created by liuyuzhou on 2021/9/14.
//
#include "GaussianFilter.h"
#include "LogUtil.h"
#include "Common.h"

#define TAG "GaussianFilter"

GaussianFilter::GaussianFilter() {
    mHorFilter = new GaussianOnePassFilter(Orientation::HORIZONTAL);
    mVerFilter = new GaussianOnePassFilter(Orientation::VERTICAL);
    mFilterType = new char[strlen(render::FILTER_TYPE_GROUP) + 1];
    std::memcpy(mFilterType, render::FILTER_TYPE_GROUP, strlen(render::FILTER_TYPE_GROUP));
    mFilterType[strlen(render::FILTER_TYPE_GROUP)] = 0;
}

GaussianFilter::~GaussianFilter() {
    if (mHorFilter != nullptr) {
        delete mHorFilter;
        mHorFilter = nullptr;
    }
    if (mVerFilter != nullptr) {
        delete mVerFilter;
        mVerFilter = nullptr;
    }
}

void GaussianFilter::adjustHorBlur(int progress) {
    if (progress < 0) {
        progress = 0;
    } else if(progress > 100) {
        progress = 100;
    }
    mHorBlurSize = ((float)progress) / 100 * 3;
    LogUtil::logI(TAG, {"adjustHorBlur: progress = ", std::to_string(progress), ", bluer size = ", std::to_string(mHorBlurSize)});
}

void GaussianFilter::adjustVerBlur(int progress) {
    if (progress < 0) {
        progress = 0;
    } else if(progress > 100) {
        progress = 100;
    }
    mVerBlurSize = ((float)progress) / 100 * 3;
    LogUtil::logI(TAG, {"adjustVerBlur: progress = ", std::to_string(progress), ", bluer size = ", std::to_string(mVerBlurSize)});
}

void GaussianFilter::init() {
    if (!mInitialized) {
        preInit();
        mHorFilter->init();
        mVerFilter->init();
        if (!mHorFilter->initialized() || !mVerFilter->initialized()) {
            mHorFilter->destroy();
            mVerFilter->destroy();
            LogUtil::logI(TAG, {"init: failed"});
            return;
        }
        initFrameBuffer();
        initTexture();
        postInit();
        mInitialized = true;
    }
}

void GaussianFilter::initFrameBuffer() {
    if (mFrameBufferId != 0) { glDeleteFramebuffers(1, &mFrameBufferId); }
    glGenFramebuffers(1, &mFrameBufferId);
}

void GaussianFilter::initTexture() {
    if (mTextureId != 0) { glDeleteTextures(1, &mTextureId); }
    glGenTextures(1, &mTextureId);

    glBindTexture(GL_TEXTURE_2D, mTextureId);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, mWidth, mHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE,nullptr);

    glBindFramebuffer(GL_FRAMEBUFFER, mFrameBufferId);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, mTextureId, 0);

    glBindTexture(GL_TEXTURE_2D, 0);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

GLint GaussianFilter::onDraw(GLint inputFrameBufferId, GLint inputTextureId) {
    glBindFramebuffer(GL_FRAMEBUFFER, mFrameBufferId);
    mHorFilter->adjustHorBlur(mHorBlurSize);
    mHorFilter->onDraw(inputTextureId);
    inputTextureId = mTextureId;

    glBindFramebuffer(GL_FRAMEBUFFER, inputFrameBufferId);
    mVerFilter->adjustVerBlur(mVerBlurSize);
    mVerFilter->flip(false, true);
    mVerFilter->onDraw(inputTextureId);
    //the return value is not used
    return inputTextureId;
}

void GaussianFilter::onPause() {
    BaseFilter::onPause();
    mHorFilter->onPause();
    mVerFilter->onPause();
}

void GaussianFilter::destroy() {
    if (mHorFilter != nullptr) { mHorFilter->destroy(); }
    if (mVerFilter != nullptr) { mVerFilter->destroy(); }
    if (mFrameBufferId != 0) {
        glDeleteFramebuffers(1, &mFrameBufferId);
        mFrameBufferId = 0;
    }
    if (mTextureId != 0) {
        glDeleteTextures(1, &mTextureId);
        mTextureId = 0;
    }
}

void GaussianFilter::setOutputSize(GLint width, GLint height) {
    BaseFilter::setOutputSize(width, height);
    mHorFilter->setOutputSize(width, height);
    mVerFilter->setOutputSize(width, height);
}
