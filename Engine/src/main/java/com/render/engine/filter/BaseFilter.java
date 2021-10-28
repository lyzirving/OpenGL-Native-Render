package com.render.engine.filter;

import androidx.annotation.NonNull;

import com.render.engine.core.BaseRenderEngine;
import com.render.engine.util.LogUtil;

public class BaseFilter {
    private static final String TAG = "BaseFilter";
    protected BaseRenderEngine mRenderEngine;
    @FilterConst.FilterType
    protected String mType;

    public BaseFilter(@FilterConst.FilterType String type) {
        mType = type;
    }

    public final void adjust(int progress) {
        if (mRenderEngine == null) {
            LogUtil.e(TAG, "adjust: engine is null");
            return;
        }
        if (!mRenderEngine.isInitialized()) {
            LogUtil.e(TAG, "adjust: render env is not initialized, filter type = " + mType);
            return;
        }
        mRenderEngine.adjust(mType, progress);
    }

    public final void adjustProperty(@NonNull String property, int progress) {
        if (mRenderEngine == null) {
            LogUtil.e(TAG, "adjustProperty: engine is null");
            return;
        }
        if (!mRenderEngine.isInitialized()) {
            LogUtil.e(TAG, "adjustProperty: render env is not initialized, filter type = " + mType);
            return;
        }
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
