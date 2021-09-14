package com.render.engine.core.filter;

import com.render.engine.core.RenderEngine;

public class ExposureFilter extends RenderEngine.BaseFilter {
    @Override
    public String getType() {
        return FilterConst.EXPOSURE;
    }
}
