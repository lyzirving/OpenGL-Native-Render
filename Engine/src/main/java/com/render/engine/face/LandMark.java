package com.render.engine.face;

import android.graphics.PointF;

import androidx.annotation.NonNull;

/**
 * @author lyzirving
 */
public class LandMark implements Cloneable {
    public static final int POINT_NUM = 68;

    private PointF[] mPoints;
    public LandMark() { mPoints = new PointF[POINT_NUM]; }

    public PointF getPoint(int index) {
        if (index >= POINT_NUM) { throw new RuntimeException("input index is out of range"); }
        if (mPoints[index] == null) { mPoints[index] = new PointF(); }
        return mPoints[index];
    }

    public void setPoint(int index, float x, float y) {
        if (index >= POINT_NUM) { throw new RuntimeException("input index is out of range"); }
        if (mPoints[index] == null) { mPoints[index] = new PointF(); }
        mPoints[index].x = x;
        mPoints[index].y = y;
    }


    @NonNull
    @Override
    protected Object clone() throws CloneNotSupportedException {
        LandMark clone = new LandMark();
        for (int i = 0; i < POINT_NUM; i++) {
            clone.setPoint(i, mPoints[i].x, mPoints[i].y);
        }
        return clone;
    }
}
