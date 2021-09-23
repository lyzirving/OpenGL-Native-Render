package com.render.engine.core;

import android.view.Surface;

public interface IRenderEngine {
    boolean isInitialized();
    void onPause();
    void onResume(Surface surface);
    void onSurfaceCreate(Surface surface, RenderAdapter adapter);
    void onSurfaceChange(int width, int height);
    void onPreviewChange(int previewWidth, int previewHeight);
    void requestRender();
    void release();
}
