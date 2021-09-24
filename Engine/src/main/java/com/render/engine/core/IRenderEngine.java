package com.render.engine.core;

import android.view.Surface;

import com.render.engine.filter.BaseFilter;

public interface IRenderEngine {
    void adjust(String filterType, int progress);
    void adjustProperty(String filterType, String property, int progress);
    void addBeautyFilter(BaseFilter filter, boolean initRightNow);
    void clearBeautyFilter();
    boolean isInitialized();
    void onPause();
    void onResume(Surface surface);
    void onSurfaceCreate(Surface surface, RenderAdapter adapter);
    void onSurfaceChange(int width, int height);
    void onPreviewChange(int previewWidth, int previewHeight);
    void requestRender();
    void release();
}
