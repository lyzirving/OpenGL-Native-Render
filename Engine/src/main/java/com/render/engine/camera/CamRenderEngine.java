package com.render.engine.camera;

import android.graphics.SurfaceTexture;
import android.hardware.camera2.CameraMetadata;
import android.view.Surface;

import androidx.annotation.NonNull;

import com.render.engine.core.BaseRenderEngine;
import com.render.engine.core.RenderAdapter;
import com.render.engine.filter.BaseFilter;
import com.render.engine.util.LogUtil;

import java.util.Map;

public class CamRenderEngine extends BaseRenderEngine {
    private static final String TAG = "CamRenderEngine";

    public CamRenderEngine() {
        mNativePtr = nCreate();
    }

    @Override
    public void addBeautyFilter(@NonNull BaseFilter filter, boolean commit) {
        if (!isInitialized()) { throw new RuntimeException(TAG + "addBeautyFilter: env is not initialized, filter = " + filter.getType()); }
        if (nAddBeautyFilter(mNativePtr, filter.getType(), commit)) {
            filter.setRenderEngine(this);
            mBeautyFilter.put(filter.getType(), filter);
        }
    }

    @Override
    public void adjust(@NonNull String filterType, int progress) {
        if (!isInitialized()) {
            LogUtil.i(TAG, "adjust: env is not initialized");
            return;
        }
        nAdjust(mNativePtr, filterType, progress);
    }

    public void buildTexture() {
        if (mNativePtr == INVALID_PTR) {
            LogUtil.e(TAG, "buildTexture: invalid native pointer");
            return;
        }
        nBuildTexture(mNativePtr);
    }

    @Override
    public void clearBeautyFilter() {
        if (!isInitialized()) {
            LogUtil.e(TAG, "clearBeautyFilter: invalid state");
            return;
        }
        if (!mBeautyFilter.isEmpty()) {
            for (Map.Entry<String, BaseFilter> tmp : mBeautyFilter.entrySet()) {
                tmp.getValue().release();
            }
            mBeautyFilter.clear();
        }
        nClearBeautyFilter(mNativePtr);
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
    public void onPreviewChange(int previewWidth, int previewHeight) {
        if (mNativePtr == INVALID_PTR) {
            LogUtil.e(TAG, "onPreviewChange: invalid native pointer");
            return;
        }
        //does not need to check whether the render is initialized
        nPreviewChange(mNativePtr, previewWidth, previewHeight);
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
        //does not need to check whether the render is initialized
        nSurfaceChange(mNativePtr, width, height);
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

    private static native boolean nAddBeautyFilter(long ptr, String filterType, boolean commit);
    private static native void nAdjust(long ptr, String filterType, int progress);
    private static native void nBuildTexture(long ptr);
    private static native void nClearBeautyFilter(long ptr);
    private static native long nCreate();
    private static native boolean nInitialized(long ptr);
    private static native void nOnPause(long ptr);
    private static native void nOnResume(long ptr, Surface surface);
    private static native void nPreviewChange(long ptr, int previewWidth, int previewHeight);
    private static native void nRequestRender(long ptr);
    private static native void nRelease(long ptr);
    private static native void nSurfaceCreate(long ptr, Surface surface, RenderAdapter adapter);
    private static native void nSurfaceChange(long ptr, int width, int height);
    private static native void nSetRenderCamMetadata(long ptr, RenderCamMetadata data);
    private static native void nSetSurfaceTexture(long ptr, SurfaceTexture surfaceTexture);
}
