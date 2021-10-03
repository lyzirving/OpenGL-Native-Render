package com.render.demo.ui;

import android.content.Context;
import android.graphics.Canvas;
import android.graphics.Color;
import android.graphics.Paint;
import android.graphics.RectF;
import android.util.AttributeSet;
import android.view.View;

import com.render.engine.util.ComponentUtil;

import androidx.annotation.Nullable;

/**
 * @author lyzirving
 */
public class FaceRenderView extends View {

    private RectF[] mFaces;
    private Paint mPaint;

    public FaceRenderView(Context context) {
        super(context, null);
    }

    public FaceRenderView(Context context, @Nullable AttributeSet attrs) {
        this(context, attrs, 0);
    }

    public FaceRenderView(Context context, @Nullable AttributeSet attrs, int defStyleAttr) {
        super(context, attrs, defStyleAttr);
        init();
    }

    @Override
    protected void onDraw(Canvas canvas) {
        super.onDraw(canvas);
        if (mFaces != null && mFaces.length > 0) {
            for (RectF face : mFaces) { canvas.drawRect(face, mPaint); }
        }
    }

    public void setFaces(RectF[] faces) {
        mFaces = faces;
        invalidate();
    }

    private void init() {
        mPaint = new Paint(Paint.ANTI_ALIAS_FLAG);
        mPaint.setColor(Color.parseColor("#ff0e00"));
        mPaint.setStyle(Paint.Style.STROKE);
        mPaint.setStrokeWidth(ComponentUtil.get().dp2px(3));
    }
}
