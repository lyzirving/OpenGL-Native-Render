package com.render.engine.core;

import android.content.Context;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.view.Surface;

import androidx.annotation.DrawableRes;

import com.render.engine.core.filter.FilterConst;
import com.render.engine.util.BitmapUtil;
import com.render.engine.util.LogUtil;

import java.util.LinkedHashMap;
import java.util.Map;

public class RenderEngine implements IRenderEngine {
    private static final String TAG = "RenderEngine";
    private static final long INVALID_PTR = -1;
    private int mSurfaceWidth, mSurfaceHeight;

    private long mNativePtr;
    private LinkedHashMap<String, BaseFilter> mFilters = new LinkedHashMap<>();

    public RenderEngine() { mNativePtr = nCreate(); }

    public void addBeautyFilter(BaseFilter filter) {
        addBeautyFilter(filter, true);
    }

    public void addBeautyFilter(BaseFilter filter, boolean buildInitTask) {
        if (!isInitialized()) {
            LogUtil.e(TAG, "addBeautyFilter: invalid state, type = " + filter.getType());
            return;
        }
        if (nAddBeautyFilter(mNativePtr, filter.getType(), buildInitTask)) {
            filter.setRenderEnvPtr(mNativePtr);
            //add or update
            mFilters.put(filter.getType(), filter);
        }
    }

    public void clearBeautyFilter() {
        if (!isInitialized()) {
            LogUtil.e(TAG, "clearBeautyFilter: invalid state");
            return;
        }
        mFilters.clear();
        nClearBeautyFilter(mNativePtr);
    }

    @Override
    public boolean isInitialized() {
        if (mNativePtr == INVALID_PTR) {
            LogUtil.e(TAG, "isInitialized: invalid native pointer");
            return false;
        }
        boolean res = nIsEnvInitialized(mNativePtr);
        LogUtil.i(TAG, "isInitialized: " + res);
        return res;
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
        if (mNativePtr == INVALID_PTR) {
            LogUtil.e(TAG, "requestRender: invalid native pointer");
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
        if (!mFilters.isEmpty()) {
            for(Map.Entry<String, BaseFilter> entry : mFilters.entrySet()) {
                entry.getValue().setRenderEnvPtr(INVALID_PTR);
            }
            mFilters.clear();
        }
    }

    public void setResource(Context ctx, @DrawableRes int id) {
        if (mSurfaceWidth == 0 || mSurfaceHeight == 0) {
            LogUtil.i(TAG, "setResource: size is invalid");
            return;
        }
        Bitmap bitmap = BitmapFactory.decodeResource(ctx.getResources(), id);
        Bitmap target = BitmapUtil.scaleBitmapToFit(bitmap, mSurfaceWidth, mSurfaceHeight);
        nSetResource(mNativePtr, target);
        if (bitmap == target) {
            bitmap.recycle();
        } else {
            bitmap.recycle();
            target.recycle();
        }
    }

    public abstract static class BaseFilter {
        protected long mRenderEnvPtr = INVALID_PTR;

        public final void adjust(int progress) {
            if (!nIsEnvInitialized(mRenderEnvPtr)) {
                LogUtil.i("BaseFilter", "adjust: env is not initialized");
                return;
            }
            nAdjust(mRenderEnvPtr, getType(), progress);
        }

        public final void adjustProp(@FilterConst.FilterProp String propName, int progress) {
            if (!nIsEnvInitialized(mRenderEnvPtr)) {
                LogUtil.i("BaseFilter", "adjustFilterProp: env is not initialized");
                return;
            }
            nAdjustProp(mRenderEnvPtr, getType(), propName, progress);
        }

        public final boolean filterValid() { return mRenderEnvPtr != INVALID_PTR; }

        abstract public @FilterConst.FilterType String getType();

        private void setRenderEnvPtr(long ptr) { mRenderEnvPtr = ptr; }
    }

    private static native long nCreate();
    private static native boolean nAddBeautyFilter(long ptr, String filterType, boolean buildInitTask);
    private static native void nAdjust(long ptr, String filterType, int progress);
    private static native void nAdjustProp(long ptr, String filterType, String filterProp, int progress);
    private static native void nClearBeautyFilter(long ptr);
    private static native boolean nIsEnvInitialized(long ptr);
    private static native void nOnPause(long ptr);
    private static native void nOnResume(long ptr, Surface surface);
    private static native void nRelease(long ptr);
    private static native void nRequestRender(long ptr);
    private static native void nSetResource(long ptr, Bitmap bmp);
    private static native void nSurfaceCreate(long ptr, Surface surface, RenderAdapter adapter);
    private static native void nSurfaceChange(long ptr, int width, int height);
}
