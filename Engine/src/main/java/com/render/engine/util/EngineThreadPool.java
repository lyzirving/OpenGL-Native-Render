package com.render.engine.util;

import java.util.concurrent.Executors;
import java.util.concurrent.SynchronousQueue;
import java.util.concurrent.ThreadFactory;
import java.util.concurrent.ThreadPoolExecutor;
import java.util.concurrent.TimeUnit;
import java.util.concurrent.atomic.AtomicInteger;

public class EngineThreadPool {
    private static final String TAG = "EngineThreadPool";
    private static volatile EngineThreadPool sInstance;

    private ThreadPoolExecutor mCachedThreadPool;

    private EngineThreadPool() {}

    public static EngineThreadPool get() {
        if (sInstance == null) {
            synchronized (EngineThreadPool.class) {
                if (sInstance == null) {
                    sInstance = new EngineThreadPool();
                }
            }
        }
        return sInstance;
    }

    public void execute(Runnable task) {
        init();
        mCachedThreadPool.execute(task);
    }

    public void init() {
        if (mCachedThreadPool == null) {
            mCachedThreadPool = new ThreadPoolExecutor(0, Integer.MAX_VALUE, 60L,
                    TimeUnit.SECONDS, new SynchronousQueue<Runnable>(), new PoolThreadFactory(TAG));
        }
    }

    public void release() {
        if (mCachedThreadPool != null) {
            mCachedThreadPool.shutdown();
        }
        mCachedThreadPool = null;
    }

    private static class PoolThreadFactory implements ThreadFactory {
        private final ThreadFactory mDefaultThreadFactory;
        private final String mBaseName;
        private final AtomicInteger mCount = new AtomicInteger(0);

        public PoolThreadFactory(final String baseName) {
            mDefaultThreadFactory = Executors.defaultThreadFactory();
            mBaseName = baseName;
        }

        @Override
        public Thread newThread(final Runnable runnable) {
            final Thread thread = mDefaultThreadFactory.newThread(runnable);
            thread.setName(mBaseName + "-" + mCount.getAndIncrement());
            return thread;
        }
    }
}
