package com.render.engine.util;

import android.util.Log;

/**
 * @author lyzirving
 */
public class LogUtil {
    private static final String TAG = "Engine";

    public static void d(String tag, String msg) {
        Log.d(buildTag(tag), msg);
    }

    public static void i(String tag, String msg) { Log.i(buildTag(tag), msg); }

    public static void e(String tag, String msg) {
        Log.e(buildTag(tag), msg);
    }

    private static String buildTag(String tag) {
        return TAG + "_" + tag;
    }
}
