//
// Created by liuyuzhou on 2021/9/14.
//
#ifndef ENGINE_HIGHLIGHTSHADOWFILTER_H
#define ENGINE_HIGHLIGHTSHADOWFILTER_H

#include "BaseFilter.h"

/**
 * shadows: Increase to lighten shadows, from 0.0 to 1.0, with 0.0 as the default.
 * highlights: Decrease to darken highlights, from 0.0 to 1.0, with 1.0 as the default.
 */
class HighlightShadowFilter : public BaseFilter {
public:

    void adjustProperty(const char *property, int progress) override;
    void init() override;
    void initHandler() override;
    void loadShader() override;
    GLint onDraw(GLint inputTextureId) override;

private:
    int mShadowHandler{0};
    int mHighlightHandler{0};
    float mShadow{0};
    float mHighlight{1};
};

#endif //ENGINE_HIGHLIGHTSHADOWFILTER_H
