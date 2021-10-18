//
// Created by liuyuzhou on 2021/9/8.
//
#ifndef ENGINE_BASEFILTERGROUP_H
#define ENGINE_BASEFILTERGROUP_H

#include "BaseFilter.h"
#include <map>

class BaseFilterGroup : public BaseFilter {
public:
    BaseFilterGroup();
    ~BaseFilterGroup();

    void addFilter(const std::string& key, std::shared_ptr<BaseFilter> filter);
    bool containsFilter(const std::string& key);
    void destroy() override;
    int filterSize();
    std::shared_ptr<BaseFilter> getFilter(const std::string& key);
    void init() override;
    void initFrameBuffer() override;
    void initTexture() override;
    GLint onDraw(GLint inputTextureId) override;
    void onPause() override;
    void onResume() override;
    std::shared_ptr<BaseFilter> removeFilter(const std::string& key);
    void setOutputSize(GLint width, GLint height) override;

private:
    std::map<std::string, std::shared_ptr<BaseFilter>>* mFilters = nullptr;
    GLuint* mFrameBuffer = nullptr;
    int mFrameBufferSize{0};
    GLuint* mFrameBufferTextures = nullptr;
    int mFrameBufferTextureSize{0};
};

#endif //ENGINE_BASEFILTERGROUP_H
