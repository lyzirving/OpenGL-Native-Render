package com.render.engine.core;

import android.graphics.RectF;

import com.render.engine.face.LandMark;

/**
 * Created by liuyuzhou
 * Created on 2021/8/30
 */
class RenderListener implements IRenderListener {
    @Override
    public void onFaceDetect(RectF[] faces) {}

    @Override
    public void onLandmarkDetect(LandMark[] landMarks) {}

    @Override
    public void onNoFaceDetect() {}

    @Override
    public void onRenderEnvPrepare() {}

    @Override
    public void onRenderEnvRelease() {}

    @Override
    public void onRenderOesTextureCreate(int oesTexture) {}

    @Override
    public void onTrackStart() {}

    @Override
    public void onTrackStop() {}
}
