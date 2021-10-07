package com.render.engine.core;

import android.graphics.RectF;

import com.render.engine.face.LandMark;

/**
 * Created on 2021/8/30
 * @author liuyuzhou
 */
public interface IRenderListener {
    /**
     * called when detect faces
     * @param faces RectF which contains faces
     */
    void onFaceDetect(RectF[] faces);

    /**
     * called when land marks are detected
     * @param landMarks
     */
    void onLandmarkDetect(LandMark[] landMarks);

    /**
     * called when no faces are detected
     */
    void onNoFaceDetect();

    /**
     * called when the render env is prepared
     */
    void onRenderEnvPrepare();

    /**
     * called when the render env is released
     */
    void onRenderEnvRelease();

    /**
     * called when the render oes texture is created
     * @param oesTexture textureId
     */
    void onRenderOesTextureCreate(int oesTexture);

    /**
     * called when the render starts to track
     */
    void onTrackStart();

    /**
     * called when the render stop to track
     */
    void onTrackStop();
}
