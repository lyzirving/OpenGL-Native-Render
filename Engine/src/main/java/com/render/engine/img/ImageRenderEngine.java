package com.render.engine.img;

import android.content.Context;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;

import com.render.engine.core.BaseRenderEngine;
import com.render.engine.util.BitmapUtil;
import com.render.engine.util.LogUtil;

import androidx.annotation.DrawableRes;

/**
 * @author lyzirving
 */
public class ImageRenderEngine extends BaseRenderEngine {
    private static final String TAG = "ImageRenderEngine";

    public ImageRenderEngine() { mNativePtr = nCreate(); }

    public void setResource(Context ctx, @DrawableRes int id) {
        if (mSurfaceWidth == 0 || mSurfaceHeight == 0) { throw new RuntimeException("surface is not ready yet"); }
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

    private static native long nCreate();
    private static native void nSetResource(long ptr, Bitmap bmp);
}
