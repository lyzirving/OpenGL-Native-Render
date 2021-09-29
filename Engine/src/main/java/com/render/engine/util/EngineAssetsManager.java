package com.render.engine.util;

import android.content.Context;
import android.media.MediaMetadataRetriever;
import android.os.Environment;
import android.text.TextUtils;

import androidx.annotation.NonNull;

import java.io.File;
import java.io.FileOutputStream;
import java.io.InputStream;
import java.util.ArrayList;
import java.util.List;
import java.util.concurrent.Executors;
import java.util.concurrent.SynchronousQueue;
import java.util.concurrent.ThreadFactory;
import java.util.concurrent.ThreadPoolExecutor;
import java.util.concurrent.TimeUnit;
import java.util.concurrent.atomic.AtomicInteger;

/**
 * @author lyzirving
 */
public class EngineAssetsManager {
    private static final String TAG = "EngineAssetsManager";
    private static volatile EngineAssetsManager sInstance;
    private static final int END_OF_STREAM = -1;

    private ThreadPoolExecutor mCachedThreadPool;
    private AssetsListener mListener;

    private EngineAssetsManager() {
        buildThreadPoolIfNeed();
    }

    public static EngineAssetsManager get() {
        if (sInstance == null) {
            synchronized (EngineAssetsManager.class) {
                if (sInstance == null) {
                    sInstance = new EngineAssetsManager();
                }
            }
        }
        return sInstance;
    }

    public void copyAssets(final Context ctx, final AssetsType type) {
        buildThreadPoolIfNeed();
        mCachedThreadPool.execute(new Runnable() {
            @Override
            public void run() {
                try {
                    String[] fileNames = ctx.getAssets().list(getAssetsTypeStr(type));
                    if (fileNames == null || fileNames.length == 0) {
                        LogUtil.e(TAG, "copyAssets: open failed, " + getAssetsTypeStr(type) + " is empty");
                        return;
                    }
                    List<MediaInfo> mediaInfo = new ArrayList<>();
                    MediaInfo tmp;
                    boolean isMedia = (type == AssetsType.MUSIC || type == AssetsType.VIDEO);
                    String rootDir = getAppAssetsDirectory(ctx, type);
                    if (TextUtils.isEmpty(rootDir)) {
                        LogUtil.e(TAG, "copyAssets: failed to find root directory of type " + getAssetsTypeStr(type));
                        return;
                    }
                    for (String fileName : fileNames) {
                        if (copyAssetsInner(ctx, type, rootDir, fileName) && isMedia) {
                            tmp = getMediaInfo(type, rootDir, fileName);
                            if (tmp != null) { mediaInfo.add(tmp); }
                        }
                    }
                    if (mediaInfo.size() != 0 && mListener != null) { mListener.onMediaDetected(mediaInfo); }
                } catch (Exception e) {
                    LogUtil.e(TAG, "copyAssets: exception = " + e.getMessage());
                    e.printStackTrace();
                }
            }
        });
    }

    public void destroy() {
        if (mCachedThreadPool != null) {
            mCachedThreadPool.shutdown();
            mCachedThreadPool = null;
        }
        mListener = null;
    }

    public void executeAsyncTask(Runnable task) {
        buildThreadPoolIfNeed();
        mCachedThreadPool.execute(task);
    }

    public void setAssetsListener(AssetsListener listener) {
        mListener = listener;
    }

    private void buildThreadPoolIfNeed() {
        if (mCachedThreadPool == null) {
            mCachedThreadPool = new ThreadPoolExecutor(
                    0, Integer.MAX_VALUE,
                    60L, TimeUnit.SECONDS,
                    new SynchronousQueue<Runnable>(),
                    new AssetsThreadFactory(TAG),
                    new ThreadPoolExecutor.AbortPolicy());
        }
    }

    private boolean copyAssetsInner(Context ctx, AssetsType type, @NonNull String rootDir, @NonNull String name) {
        File root = new File(rootDir);
        if (!root.exists()) { root.mkdirs(); }
        File dst = new File(root, name);
        if (dst.exists()) {
            LogUtil.i(TAG, "copyAssetsInner: " + dst.getAbsolutePath() + " already exists");
            return true;
        }
        try {
            InputStream is = ctx.getAssets().open(getAssetsTypeStr(type) + "/" + name);
            FileOutputStream fos = new FileOutputStream(dst);
            byte[] buffer = new byte[1024];
            int byteCount;
            while ((byteCount = is.read(buffer)) != END_OF_STREAM) {
                fos.write(buffer, 0, byteCount);
            }
            fos.flush();
            is.close();
            fos.close();
        } catch (Exception e) {
            LogUtil.d(TAG, "copyAssetsInner: exception = " + e.getMessage());
            e.printStackTrace();
            return false;
        }
        LogUtil.i(TAG, "copyAssetsInner: succeed to copy" + dst.getAbsolutePath());
        return true;
    }

    private String getAppAssetsDirectory(Context ctx, AssetsType type) {
        File root = null;
        switch (type) {
            case MUSIC: {
                root = ctx.getExternalFilesDir(Environment.DIRECTORY_MUSIC);
                break;
            }
            case VIDEO: {
                root = ctx.getExternalFilesDir(Environment.DIRECTORY_MOVIES);
                break;
            }
            case MODEL:
            case LANDMARK:
            case CLASSIFIER: {
                root = ctx.getExternalFilesDir(Environment.DIRECTORY_DOCUMENTS);
                break;
            }
            default: {
               break;
            }
        }
        return root != null ? root.getAbsolutePath() : "";
    }

    private String getAssetsTypeStr(AssetsType type) {
        switch (type) {
            case MUSIC: {
                return "music";
            }
            case VIDEO: {
                return "video";
            }
            case CLASSIFIER: {
                return "classifier";
            }
            case LANDMARK: {
                return "landmark";
            }
            case MODEL: {
                return "model";
            }
            default: {
                return "";
            }
        }
    }

    private MediaInfo getMediaInfo(AssetsType type, @NonNull String rootDir, @NonNull String name) {
        try {
            MediaMetadataRetriever retriever = new MediaMetadataRetriever();
            String path = rootDir + "/" + name;
            retriever.setDataSource(path);
            MediaInfo info = new MediaInfo(type);
            String durationStr = retriever.extractMetadata(MediaMetadataRetriever.METADATA_KEY_DURATION);
            info.name = name;
            info.nameWithoutSuffix = name.contains(".") ? name.substring(0, name.indexOf(".")) : name;
            info.duration = (TextUtils.isEmpty(durationStr) ? 0 : Integer.parseInt(durationStr)) / 1000;
            info.path = path;
            LogUtil.d(TAG, "getMediaInfo: " + info);
            return info;
        } catch (Exception e) {
            LogUtil.d(TAG, "getMediaInfo: " + name + ", exception = " + e.getMessage());
            e.printStackTrace();
            return null;
        }
    }

    public enum AssetsType {
        /**
         * music source
         */
        MUSIC,
        /**
         * video source
         */
        VIDEO,
        /**
         * classifier
         */
        CLASSIFIER,
        /**
         * landmark
         */
        LANDMARK,
        /**
         * model
         */
        MODEL
    }

    public interface AssetsListener {
        /**
         * called when medias are detected
         * @param list information of detected media list
         */
        void onMediaDetected(List<MediaInfo> list);
    }

    private static class AssetsThreadFactory implements ThreadFactory {
        private final ThreadFactory mDefaultThreadFactory;
        private final String mBaseName;
        private final AtomicInteger mCount = new AtomicInteger(0);

        public AssetsThreadFactory(final String baseName) {
            mDefaultThreadFactory = Executors.defaultThreadFactory();
            mBaseName = baseName;
        }

        @Override
        public Thread newThread(Runnable r) {
            final Thread thread = mDefaultThreadFactory.newThread(r);
            thread.setName(mBaseName + "-" + mCount.getAndIncrement());
            return thread;
        }
    }
}
