//
// Created by liuyuzhou on 2021/9/14.
//
#ifndef ENGINE_EXPOSUREFILTER_H
#define ENGINE_EXPOSUREFILTER_H

#include "BaseFilter.h"

/**
 * exposure: The adjusted exposure (-10.0 - 10.0, with 0.0 as the default)
 * we only use [0, 2] in our case, and do not use the negative side
 */
class ExposureFilter : public BaseFilter {
public:

    void adjust(int progress) override;
    void initHandler() override;
    void loadShader() override;
    GLint onDraw(GLint inputTextureId) override;

private:
    int mExposureHandler{0};
    float mExposure{0};
};

#endif //ENGINE_EXPOSUREFILTER_H
