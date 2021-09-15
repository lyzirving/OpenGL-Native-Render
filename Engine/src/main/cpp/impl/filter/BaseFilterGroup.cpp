//
// Created by liuyuzhou on 2021/9/8.
//
#include "BaseFilterGroup.h"
#include "LogUtil.h"
#include "GaussianFilter.h"

#define TAG "BaseFilterGroup"

BaseFilterGroup::BaseFilterGroup() {
    mFilters = new std::map<std::string, std::shared_ptr<BaseFilter>>;
}

BaseFilterGroup::~BaseFilterGroup() = default;

void BaseFilterGroup::addFilter(const std::string& key, std::shared_ptr<BaseFilter> filter) {
    mFilters->insert(std::make_pair(key, filter));
}

bool BaseFilterGroup::containsFilter(const std::string &key) {
    auto item = mFilters->find(key);
    if (item != mFilters->end()) {
        LogUtil::logI(TAG, {"containsFilter: already contains filter ", key});
        return true;
    }
    return false;
}

void BaseFilterGroup::destroy() {
    LogUtil::logI(TAG, {"destroy"});
    if (mFrameBuffer != nullptr) {
        glDeleteFramebuffers(mFrameBufferSize, mFrameBuffer);
        delete[] mFrameBuffer;
        mFrameBuffer = nullptr;
    }
    mFrameBufferSize = 0;
    if (mFrameBufferTextures != nullptr) {
        glDeleteTextures(mFrameBufferTextureSize, mFrameBufferTextures);
        delete[] mFrameBufferTextures;
        mFrameBufferTextures = nullptr;
    }
    mFrameBufferTextureSize = 0;
    std::map<std::string, std::shared_ptr<BaseFilter>>::iterator iterator;
    for (iterator = mFilters->begin(); iterator != mFilters->end();) {
        std::shared_ptr<BaseFilter> filter = iterator->second;
        filter->destroy();
        iterator = mFilters->erase(iterator);
    }
    mInitialized = false;
}

int BaseFilterGroup::filterSize() {
    return mFilters != nullptr ? mFilters->size() : 0;
}

std::shared_ptr<BaseFilter> BaseFilterGroup::getFilter(const std::string &key) {
    auto item = mFilters->find(key);
    if (item != mFilters->end()) {
        return item->second;
    }
    return nullptr;
}

void BaseFilterGroup::init() {
    int filterSize = mFilters != nullptr ? (int)(mFilters->size()) : 0;
    LogUtil::logI(TAG, {"init: filter size = ", std::to_string(filterSize),
                        ", frame buffer size = ", std::to_string(mFrameBufferSize),
                        ", texture size = ", std::to_string(mFrameBufferTextureSize),
                        ", initialized = ", std::to_string(mInitialized)});
    if (filterSize != mFrameBufferSize || !mInitialized) {
        initFrameBuffer();
        initTexture();
    }
    std::map<std::string, std::shared_ptr<BaseFilter>>::iterator iterator;
    for (iterator = mFilters->begin(); iterator != mFilters->end(); iterator++) {
        LogUtil::logI(TAG, {"init: filter name ", iterator->first});
        iterator->second->init();
    }
    mInitialized = true;
}

void BaseFilterGroup::initFrameBuffer() {
    int filterSize = mFilters != nullptr ? (int)(mFilters->size()) : 0;
    if (filterSize > 0) {
        LogUtil::logI(TAG, {"initFrameBuffer: size = ", std::to_string(filterSize)});
        if (mFrameBuffer != nullptr && mFrameBufferSize > 0) {
            glDeleteFramebuffers(mFrameBufferSize, mFrameBuffer);
            delete[] mFrameBuffer;
            mFrameBuffer = nullptr;
        }
        mFrameBuffer = new GLuint[filterSize];
        glGenFramebuffers(filterSize, mFrameBuffer);
        mFrameBufferSize = filterSize;
    } else {
        LogUtil::logI(TAG, {"initFrameBuffer: no filter exists yet"});
    }
}

void BaseFilterGroup::initTexture() {
    int filterSize = mFilters != nullptr ? (int)(mFilters->size()) : 0;
    if (filterSize > 0) {
        LogUtil::logI(TAG, {"initTexture: size = ", std::to_string(filterSize)});
        if (mFrameBufferTextures != nullptr && mFrameBufferTextureSize > 0) {
            glDeleteTextures(mFrameBufferTextureSize, mFrameBufferTextures);
            delete[] mFrameBufferTextures;
            mFrameBufferTextures = nullptr;
        }
        mFrameBufferTextures = new GLuint[filterSize];
        glGenTextures(filterSize, mFrameBufferTextures);
        mFrameBufferTextureSize = filterSize;
        for (int i = 0; i < filterSize; ++i) {
            glBindTexture(GL_TEXTURE_2D, mFrameBufferTextures[i]);
            glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, mWidth, mHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE,nullptr);

            glBindFramebuffer(GL_FRAMEBUFFER, mFrameBuffer[i]);
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, mFrameBufferTextures[i], 0);

            glBindTexture(GL_TEXTURE_2D, 0);
            glBindFramebuffer(GL_FRAMEBUFFER, 0);
        }
    } else {
        LogUtil::logI(TAG, {"initTexture: no filter exists yet"});
    }
}

GLint BaseFilterGroup::onDraw(GLint inputTextureId) {
    std::map<std::string, std::shared_ptr<BaseFilter>>::iterator iterator;
    int index = 0;
    for (iterator = mFilters->begin(); iterator != mFilters->end(); iterator++) {
        glBindFramebuffer(GL_FRAMEBUFFER, mFrameBuffer[index]);
        if (iterator->second->getType() != nullptr && std::strcmp(iterator->second->getType(), EngineUtil::FILTER_TYPE_GROUP) == 0) {
            iterator->second->onDraw(mFrameBuffer[index], inputTextureId);
        } else {
            iterator->second->onDraw(inputTextureId);
        }
        inputTextureId = mFrameBufferTextures[index];
        index++;
    }
    return inputTextureId;
}

void BaseFilterGroup::onPause() {
    mInitialized = false;
    std::map<std::string, std::shared_ptr<BaseFilter>>::iterator iterator;
    for (iterator = mFilters->begin(); iterator != mFilters->end(); iterator++) {
        iterator->second->onPause();
    }
}

std::shared_ptr<BaseFilter> BaseFilterGroup::removeFilter(const std::string& key) {
    auto item = mFilters->find(key);
    if (item != mFilters->end()) {
        LogUtil::logI(TAG, {"removeFilter: ", key});
        std::shared_ptr<BaseFilter> filter = item->second;
        mFilters->erase(item);
        return filter;
    } else {
        LogUtil::logI(TAG, {"removeFilter: does not contain filter", key});
        return nullptr;
    }
}

void BaseFilterGroup::setOutputSize(GLint width, GLint height) {
    mWidth = width;
    mHeight = height;
    std::map<std::string, std::shared_ptr<BaseFilter>>::iterator iterator;
    for (iterator = mFilters->begin(); iterator != mFilters->end(); iterator++) {
        iterator->second->setOutputSize(width, height);
    }
}

