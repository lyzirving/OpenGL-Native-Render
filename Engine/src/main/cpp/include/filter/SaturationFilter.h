//
// Created by liuyuzhou on 2021/9/13.
//
#ifndef ENGINE_SATURATIONFILTER_H
#define ENGINE_SATURATIONFILTER_H

#include "BaseFilter.h"

/**
 * saturation: The degree of saturation or desaturation to apply to the image (0.0 - 2.0, with 1.0 as the default)
 */
class SaturationFilter : public BaseFilter {
public:

    void adjust(int progress) override;
    void initHandler() override;
    void loadShader() override;
    GLint onDraw(GLint inputTextureId) override;
private:
    int mSaturationHandler{0};
    float mSaturation{1};
};

#endif //ENGINE_SATURATIONFILTER_H
