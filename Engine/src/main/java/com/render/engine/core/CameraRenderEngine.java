package com.render.engine.core;

import android.graphics.SurfaceTexture;
import android.hardware.camera2.CameraMetadata;
import android.view.Surface;

import androidx.annotation.NonNull;

import com.render.engine.camera.RenderCamMetadata;
import com.render.engine.util.LogUtil;


public class CameraRenderEngine implements IRenderEngine, SurfaceTexture.OnFrameAvailableListener {
    private static final String TAG = "CameraRenderEngine";
    private static final long INVALID_PTR = -1;

    private long mNativePtr;
    private int mSurfaceWidth, mSurfaceHeight;

    public CameraRenderEngine() {
        mNativePtr = nCreate();
    }

    @Override
    public boolean isInitialized() {
        if (mNativePtr == INVALID_PTR) {
            LogUtil.e(TAG, "isInitialized: invalid native pointer");
            return false;
        }
        return nInitialized(mNativePtr);
    }

    @Override
    public void onPause() {
        if (!isInitialized()) {
            LogUtil.i(TAG, "onPause: invalid state");
            return;
        }
        LogUtil.i(TAG, "onPause");
        nOnPause(mNativePtr);
    }

    @Override
    public void onResume(Surface surface) {
        if (!isInitialized()) {
            LogUtil.i(TAG, "onResume: invalid state");
            return;
        }
        LogUtil.i(TAG, "onResume");
        nOnResume(mNativePtr, surface);
    }

    @Override
    public void onSurfaceCreate(Surface surface, RenderAdapter adapter) {
        if (mNativePtr == INVALID_PTR) {
            LogUtil.e(TAG, "onSurfaceCreate: invalid native pointer");
            return;
        }
        if (surface == null || !surface.isValid()) {
            LogUtil.e(TAG, "onSurfaceCreate: surface is not valid");
            return;
        }
        if (adapter == null) {
            LogUtil.e(TAG, "onSurfaceCreate: adapter is null");
            return;
        }
        if (isInitialized()) {
            LogUtil.e(TAG, "onSurfaceCreate: env is already initialized");
            return;
        }
        nSurfaceCreate(mNativePtr, surface, adapter);
    }

    @Override
    public void onSurfaceChange(int width, int height) {
        if (mNativePtr == INVALID_PTR) {
            LogUtil.e(TAG, "onSurfaceChange: invalid native pointer");
            return;
        }
        mSurfaceWidth = width;
        mSurfaceHeight = height;
        //does not need to check whether the render is initialized
        nSurfaceChange(mNativePtr, width, height);
    }

    @Override
    public void onFrameAvailable(SurfaceTexture surfaceTexture) {
        requestRender();
    }

    @Override
    public void requestRender() {
        if (!isInitialized()) {
            LogUtil.e(TAG, "requestRender: env is invalid");
            return;
        }
        nRequestRender(mNativePtr);
    }

    @Override
    public void release() {
        if (!isInitialized()) {
            LogUtil.e(TAG, "release: invalid state");
            return;
        }
        nRelease(mNativePtr);
        mNativePtr = INVALID_PTR;
    }

    public void setSurfaceTexture(SurfaceTexture surfaceTexture) {
        if (!isInitialized()) {
            LogUtil.e(TAG, "setSurfaceTexture: invalid state");
            return;
        }
        nSetSurfaceTexture(mNativePtr, surfaceTexture);
    }

    public void setRenderCamMetadata(@NonNull RenderCamMetadata data) {
        if (!isInitialized()) {
            LogUtil.e(TAG, "setRenderCamMetadata: invalid state");
            return;
        }
        if (data.getFrontType() != CameraMetadata.LENS_FACING_FRONT
                && data.getFrontType() != CameraMetadata.LENS_FACING_BACK
                && data.getFrontType() != CameraMetadata.LENS_FACING_EXTERNAL) {
            throw new RuntimeException("invalid camera metadata lens-facing");
        }
        nSetRenderCamMetadata(mNativePtr, data);
    }

    private static native long nCreate();
    private static native boolean nInitialized(long ptr);
    private static native void nOnPause(long ptr);
    private static native void nOnResume(long ptr, Surface surface);
    private static native void nRequestRender(long ptr);
    private static native void nRelease(long ptr);
    private static native void nSetSurfaceTexture(long ptr, SurfaceTexture surfaceTexture);
    private static native void nSurfaceCreate(long ptr, Surface surface, RenderAdapter adapter);
    private static native void nSurfaceChange(long ptr, int width, int height);
    private static native void nSetRenderCamMetadata(long ptr, RenderCamMetadata data);
}
