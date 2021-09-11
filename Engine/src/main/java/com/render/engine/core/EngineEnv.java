package com.render.engine.core;

import android.content.res.AssetManager;

public class EngineEnv {
    private static final String TAG = "EngineEnv";
    static { System.loadLibrary("engine"); }

    public static void init(AssetManager assetManager) {
        nInitEnv(assetManager);
    }

    private static native void nInitEnv(AssetManager assetManager);
}
