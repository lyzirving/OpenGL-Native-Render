package com.render.engine.share;

import com.render.engine.core.BaseRenderEngine;

public class ShareRenderEngine extends BaseRenderEngine {
    private static final String TAG = "ShareRenderEngine";

    public ShareRenderEngine() {
        mNativePtr = nCreate();
    }

    private static native long nCreate();
}
