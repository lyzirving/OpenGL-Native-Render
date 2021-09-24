package com.render.engine.core.filter;

import android.graphics.SurfaceTexture;
import android.view.Surface;

import com.render.engine.core.IRenderEngine;
import com.render.engine.core.RenderAdapter;

public class BaseRenderEngine implements IRenderEngine, SurfaceTexture.OnFrameAvailableListener {
    protected static final long INVALID_PTR = -1;
    protected long mNativePtr = INVALID_PTR;

    @Override
    public void onFrameAvailable(SurfaceTexture surfaceTexture) {
        requestRender();
    }

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
