package com.render.engine.core;

import android.graphics.SurfaceTexture;
import android.view.Surface;

import com.render.engine.filter.BaseFilter;

import java.util.LinkedHashMap;


public class BaseRenderEngine implements IRenderEngine, SurfaceTexture.OnFrameAvailableListener {
    protected static final long INVALID_PTR = -1;
    protected long mNativePtr = INVALID_PTR;
    protected LinkedHashMap<String, BaseFilter> mBeautyFilter = new LinkedHashMap<>();

    @Override
    public void addBeautyFilter(BaseFilter filter, boolean commit) {}

    @Override
    public void adjust(String filterType, int progress) {}

    @Override
    public void adjustProperty(String filterType, String property, int progress) {}

    @Override
    public void clearBeautyFilter() {}

    @Override
    public void onFrameAvailable(SurfaceTexture surfaceTexture) { requestRender(); }

    @Override
    public boolean isInitialized() {
        return false;
    }

    @Override
    public void onPause() {}

    @Override
    public void onResume(Surface surface) {}

    @Override
    public void onSurfaceCreate(Surface surface, RenderAdapter adapter) {}

    @Override
    public void onSurfaceChange(int width, int height) {}

    @Override
    public void onPreviewChange(int previewWidth, int previewHeight) {}

    @Override
    public void requestRender() {}

    @Override
    public void release() {}
}
