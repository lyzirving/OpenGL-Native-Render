package com.render.engine.core.filter;

import com.render.engine.core.RenderEngine;
import com.render.engine.filter.FilterConst;

public class ContrastFilter extends RenderEngine.BaseFilter {

    @Override
    public String getType() { return FilterConst.CONTRAST; }
}
