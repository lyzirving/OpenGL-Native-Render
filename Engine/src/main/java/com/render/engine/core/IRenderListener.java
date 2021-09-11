package com.render.engine.core;

/**
 * Created on 2021/8/30
 * @author liuyuzhou
 */
public interface IRenderListener {
    /**
     * called when the render env is prepared
     */
    void onRenderEnvPrepare();

    /**
     * called when the render env is released
     */
    void onRenderEnvRelease();
}
