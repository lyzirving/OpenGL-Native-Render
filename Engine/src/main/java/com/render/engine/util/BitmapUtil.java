package com.render.engine.util;

import android.graphics.Bitmap;
import android.graphics.Matrix;

public class BitmapUtil {
    private static final String TAG = "BitmapUtil";

    private BitmapUtil() {}

    public static Bitmap scaleBitmapToFit(Bitmap input, int width, int height) {
        int bmpWidth = input.getWidth();
        int bmpHeight = input.getHeight();
        float bmpRatio = bmpWidth * 1f / bmpHeight;
        int dstWidth = bmpWidth, dstHeight = bmpHeight;
        if (bmpWidth > width || bmpHeight > height) {
            if (bmpRatio < 1) {
                dstHeight = height;
                dstWidth = (int) (height * bmpRatio);
            } else {
                dstWidth = width;
                dstHeight = (int) (width / bmpRatio);
                while (dstHeight > height) {
                    dstWidth -= 10;
                    dstHeight = (int) (dstWidth / bmpRatio);
                }
            }
        }
        Bitmap target = null;
        if (bmpWidth != dstWidth || bmpHeight != dstHeight) {
            LogUtil.i(TAG, "scaleBitmapToFit: rescale bmp, before width = " + input.getWidth() + ", before height = " + input.getHeight());
            Matrix matrix = new Matrix();
            matrix.setScale(dstWidth * 1f / bmpWidth, dstHeight * 1f / bmpHeight);
            target = Bitmap.createBitmap(input, 0,0, bmpWidth, bmpHeight, matrix, true);
            LogUtil.i(TAG, "scaleBitmapToFit: rescale bmp, after width = " + target.getWidth() + ", after height = " + target.getHeight());
        }
        return target != null ? target : input;
    }
}
