package com.render.engine.filter;

import androidx.annotation.NonNull;

import com.render.engine.core.BaseRenderEngine;

public class BaseFilter {
    protected BaseRenderEngine mRenderEngine;
    @FilterConst.FilterType
    protected String mType;

    public BaseFilter(@FilterConst.FilterType String type) {
        mType = type;
    }

    public final void adjust(int progress) {
        if (mRenderEngine == null) { throw new RuntimeException("adjust: render engine is null"); }
        if (!mRenderEngine.isInitialized()) { throw new RuntimeException("adjust: render env is not initialized, filter type = " + mType); }
        mRenderEngine.adjust(mType, progress);
    }

    public final void adjustProperty(@NonNull String property, int progress) {
        if (mRenderEngine == null) { throw new RuntimeException("adjustProperty: render engine is null"); }
        if (!mRenderEngine.isInitialized()) { throw new RuntimeException("adjustProperty: render env is not initialized, filter type = " + mType); }
        mRenderEngine.adjustProperty(mType, property, progress);
    }

    public final @FilterConst.FilterType String getType() {
        return mType;
    }

    public boolean filterValid() {
        return mRenderEngine != null && mRenderEngine.isInitialized();
    }

    public void release() {
        mRenderEngine = null;
        mType = null;
    }

    public void setRenderEngine(BaseRenderEngine engine) {
        mRenderEngine = engine;
    }
}
