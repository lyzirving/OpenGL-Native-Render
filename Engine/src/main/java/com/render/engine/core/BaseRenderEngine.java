package com.render.engine.core;

import android.graphics.SurfaceTexture;
import android.view.Surface;

import com.render.engine.filter.BaseFilter;
import com.render.engine.util.LogUtil;

import java.util.LinkedHashMap;
import java.util.Map;

/**
 * @author lyzirving
 */
public class BaseRenderEngine implements IRenderEngine, SurfaceTexture.OnFrameAvailableListener {
    private static final String TAG = "BaseRenderEngine";
    protected static final long INVALID_PTR = -1;

    protected long mNativePtr = INVALID_PTR;
    protected LinkedHashMap<String, BaseFilter> mBeautyFilter = new LinkedHashMap<>();
    protected int mSurfaceWidth, mSurfaceHeight;

    @Override
    public void addBeautyFilter(BaseFilter filter, boolean commit) {
        if (!isInitialized()) { throw new RuntimeException("addBeautyFilter: env is not initialized, filter = " + filter.getType()); }
        if (nAddBeautyFilter(mNativePtr, filter.getType(), commit)) {
            filter.setRenderEngine(this);
            mBeautyFilter.put(filter.getType(), filter);
        }
    }

    @Override
    public void adjust(String filterType, int progress) {
        if (!isInitialized()) {
            LogUtil.i(TAG, "adjust: env is not initialized");
            return;
        }
        nAdjust(mNativePtr, filterType, progress);
    }

    @Override
    public void adjustProperty(String filterType, String property, int progress) {
        if (!isInitialized()) {
            LogUtil.i(TAG, "adjustProperty: env is not initialized");
            return;
        }
        nAdjustProperty(mNativePtr, filterType, property, progress);
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
    public void onFrameAvailable(SurfaceTexture surfaceTexture) { requestRender(); }

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

    private static native boolean nAddBeautyFilter(long ptr, String filterType, boolean commit);
    private static native void nAdjust(long ptr, String filterType, int progress);
    private static native void nAdjustProperty(long ptr, String filterType, String property, int progress);
    private static native void nClearBeautyFilter(long ptr);
    private static native boolean nInitialized(long ptr);
    private static native void nOnPause(long ptr);
    private static native void nOnResume(long ptr, Surface surface);
    private static native void nRequestRender(long ptr);
    private static native void nRelease(long ptr);
    private static native void nSurfaceCreate(long ptr, Surface surface, RenderAdapter adapter);
    private static native void nSurfaceChange(long ptr, int width, int height);
}
