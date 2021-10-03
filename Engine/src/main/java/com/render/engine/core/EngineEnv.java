package com.render.engine.core;

import android.content.Context;
import android.content.res.AssetManager;
import android.os.Environment;

import com.render.engine.util.ComponentUtil;
import com.render.engine.util.EngineAssetsManager;
import com.render.engine.util.LogUtil;

public class EngineEnv {
    private static final String TAG = "EngineEnv";
    static { System.loadLibrary("engine"); }

    public static boolean checkSdcard() {
        boolean mounted = Environment.getExternalStorageState().equals(android.os.Environment.MEDIA_MOUNTED);
        if (mounted) {
            String sdcardPath = Environment.getExternalStorageDirectory().getAbsolutePath();
            LogUtil.i(TAG, "checkSdCard: sdcard is mounted " + sdcardPath);
        } else {
            LogUtil.e(TAG, "checkSdCard: not exist");
        }
        return mounted;
    }

    public static void init(Context ctx) {
        if (ctx == null) { throw new RuntimeException("context should not be null when render env is about to initialized"); }
        ComponentUtil.get().init(ctx);
        checkSdcard();
        EngineAssetsManager.get().copyAssets(ctx, EngineAssetsManager.AssetsType.CLASSIFIER);
        nInitEnv(ctx.getAssets());
    }

    public static void release() {
        EngineAssetsManager.get().destroy();
        ComponentUtil.get().destroy();
    }

    private static native void nInitEnv(AssetManager assetManager);
}
