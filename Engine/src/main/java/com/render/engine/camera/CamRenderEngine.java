package com.render.engine.camera;

import android.graphics.SurfaceTexture;
import android.hardware.camera2.CameraMetadata;

import androidx.annotation.NonNull;

import com.render.engine.core.BaseRenderEngine;
import com.render.engine.util.LogUtil;

/**
 * @author lyzirving
 */
public class CamRenderEngine extends BaseRenderEngine {
    private static final String TAG = "CamRenderEngine";

    public CamRenderEngine() {
        mNativePtr = nCreate();
    }

    public void buildTexture() {
        if (mNativePtr == INVALID_PTR) {
            LogUtil.e(TAG, "buildTexture: invalid native pointer");
            return;
        }
        nBuildTexture(mNativePtr);
    }

    public void detect(boolean start) {
        if (!isInitialized()) {
            LogUtil.e(TAG, "detect: invalid state");
            return;
        }
        nDetect(mNativePtr, start);
    }

    public void onPreviewChange(int previewWidth, int previewHeight) {
        if (mNativePtr == INVALID_PTR) {
            LogUtil.e(TAG, "onPreviewChange: invalid native pointer");
            return;
        }
        //does not need to check whether the render is initialized
        nPreviewChange(mNativePtr, previewWidth, previewHeight);
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

    public void setSurfaceTexture(SurfaceTexture surfaceTexture) {
        if (!isInitialized()) {
            LogUtil.e(TAG, "setSurfaceTexture: invalid state");
            return;
        }
        nSetSurfaceTexture(mNativePtr, surfaceTexture);
    }

    private static native void nBuildTexture(long ptr);
    private static native long nCreate();
    private static native void nDetect(long ptr, boolean start);
    private static native void nPreviewChange(long ptr, int previewWidth, int previewHeight);
    private static native void nSetRenderCamMetadata(long ptr, RenderCamMetadata data);
    private static native void nSetSurfaceTexture(long ptr, SurfaceTexture surfaceTexture);
}
