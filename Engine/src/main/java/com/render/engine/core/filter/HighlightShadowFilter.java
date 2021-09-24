package com.render.engine.core.filter;

import com.render.engine.core.RenderEngine;
import com.render.engine.filter.FilterConst;

public class HighlightShadowFilter extends RenderEngine.BaseFilter {
    @Override
    public String getType() {
        return FilterConst.HIGHLIGHT_SHADOW;
    }
}
