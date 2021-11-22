package com.render.engine.core;

import android.annotation.SuppressLint;
import android.graphics.SurfaceTexture;
import android.view.Surface;

import com.render.engine.util.LogUtil;

/**
 * @author lyzirving
 */
public class OffScreenRender implements SurfaceTexture.OnFrameAvailableListener {
    private static final String TAG = "OffScreenRender";
    private static final long INVALID_PTR = -1;

    protected long mNativePtr = INVALID_PTR;

    public OffScreenRender() {
        mNativePtr = nCreate();
    }

    private boolean checkPtr() { return mNativePtr != INVALID_PTR; }

    @Override
    public void onFrameAvailable(SurfaceTexture surfaceTexture) {
        nRequestRender(mNativePtr);
    }

    public void onQuit() {
        mNativePtr = INVALID_PTR;
    }

    @SuppressLint("WrongConstant")
    public void prepare(int width, int height, RenderAdapter adapter) {
        if (adapter == null) {
            LogUtil.e(TAG, "prepare: adapter is null");
            return;
        }
        if (checkPtr()) {
            nPrepare(mNativePtr, adapter);
        } else {
            LogUtil.e(TAG, "prepare: invalid pointer");
        }
    }

    public void pause() {
        if (checkPtr()) {
            nPause(mNativePtr);
        } else {
            LogUtil.e(TAG, "pause: invalid ptr");
        }
    }

    public void release() {
        if (checkPtr()) {
            nRelease(mNativePtr);
        } else {
            LogUtil.e(TAG, "release: invalid ptr");
        }
    }

    public void setClient(Surface surface) {
        if (surface == null || !surface.isValid()) {
            LogUtil.e(TAG, "setClient: surface is invalid");
            return;
        }
        if (checkPtr()) {
            nSetClient(mNativePtr, surface);
        } else {
            LogUtil.e(TAG, "setClient: invalid pointer");
        }
    }

    public void setSurfaceTexture(SurfaceTexture surfaceTexture) {
        if (surfaceTexture == null || surfaceTexture.isReleased()) {
            LogUtil.e(TAG, "setSurfaceTexture: surface texture is invalid");
            return;
        }
        if (checkPtr()) {
            nSetSurfaceTexture(mNativePtr, surfaceTexture);
        } else {
            LogUtil.e(TAG, "setSurfaceTexture: invalid pointer");
        }
    }

    private static native long nCreate();
    private static native void nPause(long ptr);
    private static native void nPrepare(long ptr, RenderAdapter adapter);
    private static native void nRequestRender(long ptr);
    private static native void nRelease(long ptr);
    private static native void nSetClient(long ptr, Surface surface);
    private static native void nSetSurfaceTexture(long ptr, SurfaceTexture surfaceTexture);
}
