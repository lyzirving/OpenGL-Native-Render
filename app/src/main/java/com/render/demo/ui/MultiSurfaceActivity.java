package com.render.demo.ui;

import android.graphics.PixelFormat;
import android.graphics.SurfaceTexture;
import android.os.Handler;
import android.os.Looper;
import android.os.Message;
import android.util.Size;
import android.view.SurfaceHolder;
import android.view.SurfaceView;

import androidx.annotation.NonNull;

import com.render.demo.R;
import com.render.engine.camera.CamRenderEngine;
import com.render.engine.camera.CameraHelper;
import com.render.engine.camera.RenderCamMetadata;
import com.render.engine.core.RenderAdapter;
import com.render.engine.filter.ColorAdjustFilter;
import com.render.engine.filter.FilterConst;
import com.render.engine.share.ShareRenderEngine;
import com.render.engine.util.LogUtil;


public class MultiSurfaceActivity extends BaseActivity {
    private static final String TAG = "MultiSurfaceActivity";

    private SurfaceHolder.Callback mMainCallback;
    private CamRenderEngine mCameraRender;
    private RenderAdapter mCamRenderAdapter;

    private final Object mShareOneLock = new Object();
    private ShareRenderEngine mShareOneRender;
    private SurfaceHolder.Callback mShareOneCallback;
    private RenderAdapter mShareOneRenderAdapter;

    private final Object mShareTwoLock = new Object();
    private ShareRenderEngine mShareTwoRender;
    private SurfaceHolder.Callback mShareTwoCallback;
    private RenderAdapter mShareTwoRenderAdapter;

    private final Object mShareTwoSubLock = new Object();
    private ShareRenderEngine mShareTwoSubRender;
    private SurfaceHolder.Callback mShareTwoSubCallback;
    private RenderAdapter mShareTwoSubRenderAdapter;

    private static final int MSG_RENDER_OES_TEXTURE_CREATE = 1;
    private Handler mMainHandler = new Handler(Looper.getMainLooper()) {
        @Override
        public void handleMessage(@NonNull Message msg) {
            super.handleMessage(msg);
            switch (msg.what) {
                case MSG_RENDER_OES_TEXTURE_CREATE: {
                    handleOesTextureCreate(msg.arg1);
                    break;
                }
                default: {
                    break;
                }
            }
        }
    };

    @Override
    protected int getLayoutId() {
        return R.layout.activity_multi_surface;
    }

    @Override
    protected void initView() {
        SurfaceView surfaceView = findViewById(R.id.surface_main);
        surfaceView.setZOrderOnTop(false);
        surfaceView.getHolder().setFormat(PixelFormat.TRANSPARENT);
        surfaceView.getHolder().addCallback(getMainCallback());

        surfaceView = findViewById(R.id.surface_share_one);
        surfaceView.setZOrderOnTop(false);
        surfaceView.getHolder().setFormat(PixelFormat.TRANSPARENT);
        surfaceView.getHolder().addCallback(getShareOneCallback());

        surfaceView = findViewById(R.id.surface_share_two);
        surfaceView.setZOrderOnTop(false);
        surfaceView.getHolder().setFormat(PixelFormat.TRANSPARENT);
        surfaceView.getHolder().addCallback(getShareTwoCallback());

        surfaceView = findViewById(R.id.surface_share_two_sub);
        surfaceView.setZOrderOnTop(false);
        surfaceView.getHolder().setFormat(PixelFormat.TRANSPARENT);
        surfaceView.getHolder().addCallback(getShareTwoSubCallback());
    }

    @Override
    protected void initData() {}

    @Override
    protected void release() {
        if (mShareOneRender != null) { mShareOneRender.release(); }
        mShareOneRender = null;

        if (mShareTwoRender != null) { mShareTwoRender.release(); }
        mShareTwoRender = null;

        if (mShareTwoSubRender != null) { mShareTwoSubRender.release(); }
        mShareTwoSubRender = null;

        if (mCameraRender != null) { mCameraRender.release(); }
        mCameraRender = null;
    }

    @Override
    protected void onPause() {
        super.onPause();
        LogUtil.i(TAG, "onPause");
        CameraHelper.get().closeCamera();
        if (mCameraRender != null) { mCameraRender.onPause(); }
        if (mShareOneRender != null) { mShareOneRender.onPause(); }
        if (mShareTwoRender != null) { mShareTwoRender.onPause(); }
        if (mShareTwoSubRender != null) { mShareTwoSubRender.onPause(); }
    }

    private SurfaceHolder.Callback getMainCallback() {
        if (mMainCallback == null) {
            mMainCallback = new SurfaceHolder.Callback() {
                @Override
                public void surfaceCreated(@NonNull SurfaceHolder holder) {
                    LogUtil.i(TAG, "surfaceCreated: main");
                    if (mCameraRender == null || !mCameraRender.isInitialized()) {
                        mCameraRender = new CamRenderEngine();
                        mCameraRender.onSurfaceCreate(holder.getSurface(), getCamRenderAdapter());
                    } else {
                        mCameraRender.onResume(holder.getSurface());
                    }
                }

                @Override
                public void surfaceChanged(@NonNull SurfaceHolder holder, int format, int width, int height) {
                    LogUtil.i(TAG, "surfaceChanged: main, width = " + width + ", height = " + height);
                    if (mCameraRender != null) {
                        CameraHelper.get().prepare(getApplicationContext(), width, height);
                        Size previewSize = CameraHelper.get().getPreviewSize();
                        RenderCamMetadata data = new RenderCamMetadata.Builder()
                                .previewSize(previewSize.getWidth(), previewSize.getHeight())
                                .frontType(CameraHelper.get().getFrontType()).build();
                        mCameraRender.setRenderCamMetadata(data);
                        //the method must be called by order onPreviewChange() -> onSurfaceChange() -> buildTexture()
                        mCameraRender.onPreviewChange(previewSize.getWidth(), previewSize.getHeight());
                        mCameraRender.onSurfaceChange(width, height);
                        mCameraRender.buildTexture();
                    }
                }

                @Override
                public void surfaceDestroyed(@NonNull SurfaceHolder holder) {
                    LogUtil.i(TAG, "surfaceDestroyed: main");
                }
            };
        }
        return mMainCallback;
    }

    private SurfaceHolder.Callback getShareOneCallback() {
        if (mShareOneCallback == null) {
            mShareOneCallback = new SurfaceHolder.Callback() {
                @Override
                public void surfaceCreated(@NonNull SurfaceHolder holder) {
                    LogUtil.i(TAG, "surfaceCreated: share one");
                    synchronized (mShareOneLock) {
                        if (mShareOneRender == null || !mShareOneRender.isInitialized()) {
                            mShareOneRender = new ShareRenderEngine();
                            mShareOneRender.onSurfaceCreate(holder.getSurface(), getShareOneRenderAdapter());
                        } else {
                            mShareOneRender.onResume(holder.getSurface());
                        }
                        mShareOneLock.notify();
                    }
                }

                @Override
                public void surfaceChanged(@NonNull SurfaceHolder holder, int format, int width, int height) {
                    LogUtil.i(TAG, "surfaceChanged: share one, width = " + width + ", height = " + height);
                    if (mShareOneRender != null) { mShareOneRender.onSurfaceChange(width, height); }
                }

                @Override
                public void surfaceDestroyed(@NonNull SurfaceHolder holder) {
                    LogUtil.i(TAG, "surfaceDestroyed: share one");
                }
            };
        }
        return mShareOneCallback;
    }

    private SurfaceHolder.Callback getShareTwoCallback() {
        if (mShareTwoCallback == null) {
            mShareTwoCallback = new SurfaceHolder.Callback() {
                @Override
                public void surfaceCreated(@NonNull SurfaceHolder holder) {
                    LogUtil.i(TAG, "surfaceCreated: share two");
                    synchronized (mShareTwoLock) {
                        if (mShareTwoRender == null || !mShareTwoRender.isInitialized()) {
                            mShareTwoRender = new ShareRenderEngine();
                            mShareTwoRender.onSurfaceCreate(holder.getSurface(), getShareTwoRenderAdapter());
                        } else {
                            mShareTwoRender.onResume(holder.getSurface());
                        }
                        mShareTwoLock.notify();
                    }
                }

                @Override
                public void surfaceChanged(@NonNull SurfaceHolder holder, int format, int width, int height) {
                    LogUtil.i(TAG, "surfaceChanged: share two, width = " + width + ", height = " + height);
                    if (mShareTwoRender != null) { mShareTwoRender.onSurfaceChange(width, height); }
                }

                @Override
                public void surfaceDestroyed(@NonNull SurfaceHolder holder) {
                    LogUtil.i(TAG, "surfaceDestroyed: share two");
                }
            };
        }
        return mShareTwoCallback;
    }

    private SurfaceHolder.Callback getShareTwoSubCallback() {
        if (mShareTwoSubCallback == null) {
            mShareTwoSubCallback = new SurfaceHolder.Callback() {
                @Override
                public void surfaceCreated(@NonNull SurfaceHolder holder) {
                    LogUtil.i(TAG, "surfaceCreated: share two sub");
                    synchronized (mShareTwoSubLock) {
                        if (mShareTwoSubRender == null || !mShareTwoSubRender.isInitialized()) {
                            mShareTwoSubRender = new ShareRenderEngine();
                            mShareTwoSubRender.onSurfaceCreate(holder.getSurface(), getShareTwoSubRenderAdapter());
                        } else {
                            mShareTwoSubRender.onResume(holder.getSurface());
                        }
                        mShareTwoSubLock.notify();
                    }
                }

                @Override
                public void surfaceChanged(@NonNull SurfaceHolder holder, int format, int width, int height) {
                    LogUtil.i(TAG, "surfaceChanged: share two sub, width = " + width + ", height = " + height);
                    if (mShareTwoSubRender != null) { mShareTwoSubRender.onSurfaceChange(width, height); }
                }

                @Override
                public void surfaceDestroyed(@NonNull SurfaceHolder holder) {
                    LogUtil.i(TAG, "surfaceDestroyed: share two sub");
                }
            };
        }
        return mShareTwoSubCallback;
    }

    private RenderAdapter getCamRenderAdapter() {
        if (mCamRenderAdapter == null) {
            mCamRenderAdapter = new RenderAdapter() {

                @Override
                public void onRenderEnvPrepare() {
                    super.onRenderEnvPrepare();
                    LogUtil.i(TAG, "onRenderEnvPrepare: camera");
                    handleCameraRenderPrepare();
                }

                @Override
                public void onRenderEnvResume() {
                    super.onRenderEnvResume();
                    LogUtil.i(TAG, "onRenderEnvResume: camera");
                    handleCameraRenderResume();
                }

                @Override
                public void onRenderEnvRelease() {
                    super.onRenderEnvRelease();
                    LogUtil.i(TAG, "onRenderEnvRelease: camera");
                    if (mCameraRender != null) { mCameraRender.abandon(); }
                    mCameraRender = null;
                }

                @Override
                public void onRenderOesTextureCreate(int oesTexture) {
                    super.onRenderOesTextureCreate(oesTexture);
                    LogUtil.i(TAG, "onRenderOesTextureCreate: " + oesTexture);
                    mMainHandler.obtainMessage(MSG_RENDER_OES_TEXTURE_CREATE, oesTexture, 0).sendToTarget();
                }
            };
        }
        return mCamRenderAdapter;
    }

    private RenderAdapter getShareOneRenderAdapter() {
        if (mShareOneRenderAdapter == null) {
            mShareOneRenderAdapter = new RenderAdapter() {
                @Override
                public void onRenderEnvPrepare() {
                    super.onRenderEnvPrepare();
                    LogUtil.i(TAG, "onRenderEnvPrepare: share one");
                    ColorAdjustFilter filter = new ColorAdjustFilter();
                    mShareOneRender.addBeautyFilter(filter, true);
                    filter.adjustProperty(FilterConst.PROP_CONTRAST, 70);
                }

                @Override
                public void onRenderEnvRelease() {
                    super.onRenderEnvRelease();
                    LogUtil.i(TAG, "onRenderEnvRelease: share one");
                    if (mShareOneRender != null) { mShareOneRender.abandon(); }
                    mShareOneRender = null;
                }
            };
        }
        return mShareOneRenderAdapter;
    }

    private RenderAdapter getShareTwoRenderAdapter() {
        if (mShareTwoRenderAdapter == null) {
            mShareTwoRenderAdapter = new RenderAdapter() {
                @Override
                public void onRenderEnvPrepare() {
                    super.onRenderEnvPrepare();
                    LogUtil.i(TAG, "onRenderEnvPrepare: share two");
                    ColorAdjustFilter filter = new ColorAdjustFilter();
                    mShareTwoRender.addBeautyFilter(filter, true);
                    filter.adjustProperty(FilterConst.PROP_SATURATION, 15);
                    handleShareTwoEnvPrepare();
                }

                @Override
                public void onRenderEnvRelease() {
                    super.onRenderEnvRelease();
                    LogUtil.i(TAG, "onRenderEnvRelease: share two");
                    if (mShareTwoRender != null) { mShareTwoRender.abandon(); }
                    mShareTwoRender = null;
                }
            };
        }
        return mShareTwoRenderAdapter;
    }

    private RenderAdapter getShareTwoSubRenderAdapter() {
        if (mShareTwoSubRenderAdapter == null) {
            mShareTwoSubRenderAdapter = new RenderAdapter() {
                @Override
                public void onRenderEnvPrepare() {
                    super.onRenderEnvPrepare();
                    LogUtil.i(TAG, "onRenderEnvPrepare: share two sub");
                }

                @Override
                public void onRenderEnvRelease() {
                    super.onRenderEnvRelease();
                    LogUtil.i(TAG, "onRenderEnvRelease: share two sub");
                    if (mShareTwoSubRender != null) { mShareTwoSubRender.abandon(); }
                    mShareTwoSubRender = null;
                }
            };
        }
        return mShareTwoSubRenderAdapter;
    }

    private void handleOesTextureCreate(int oesTexture) {
        SurfaceTexture surfaceTexture = new SurfaceTexture(oesTexture);
        mCameraRender.setSurfaceTexture(surfaceTexture);
        CameraHelper.get().setOesTexture(surfaceTexture);
        CameraHelper.get().setOnFrameAvailableListener(mCameraRender);
        CameraHelper.get().open(getApplicationContext());
    }

    private void handleCameraRenderPrepare() {
        try {
            LogUtil.i(TAG, "handleCameraRenderPrepare");
            synchronized (mShareOneLock) {
                if (mShareOneRender == null || !mShareOneRender.isInitialized()) {
                    LogUtil.i(TAG, "handleCameraRenderPrepare: share one wait");
                    mShareOneLock.wait();
                    LogUtil.i(TAG, "handleCameraRenderPrepare: share one resume");
                }
                mCameraRender.bindShareEnv(mShareOneRender.getNativePtr());
            }

            synchronized (mShareTwoLock) {
                if (mShareTwoRender == null || !mShareTwoRender.isInitialized()) {
                    LogUtil.i(TAG, "handleCameraRenderPrepare: share two wait");
                    mShareTwoLock.wait();
                    LogUtil.i(TAG, "handleCameraRenderPrepare: share two resume");
                }
                mCameraRender.bindShareEnv(mShareTwoRender.getNativePtr());
            }

        } catch (Exception e) {
            LogUtil.i(TAG, "handleCameraRenderPrepare: exception = " + e.getMessage());
            e.printStackTrace();
        }
    }

    private void handleCameraRenderResume() {
        try {
            LogUtil.i(TAG, "handleCameraRenderResume");
            synchronized (mShareOneLock) {
                if (mShareOneRender == null || !mShareOneRender.isInitialized() || !mShareOneRender.surfacePrepare()) {
                    LogUtil.i(TAG, "onRenderEnvResume: camera, wait share one");
                    mShareOneLock.wait();
                    LogUtil.i(TAG, "onRenderEnvResume: camera, resume share one");
                }
                LogUtil.i(TAG, "onRenderEnvResume: camera, bind share one");
                mCameraRender.bindShareEnv(mShareOneRender.getNativePtr());
            }

            synchronized (mShareTwoLock) {
                if (mShareTwoRender == null || !mShareTwoRender.isInitialized() || !mShareTwoRender.surfacePrepare()) {
                    LogUtil.i(TAG, "onRenderEnvResume: camera, wait share two");
                    mShareTwoLock.wait();
                    LogUtil.i(TAG, "onRenderEnvResume: camera, resume share two");
                }
                LogUtil.i(TAG, "onRenderEnvResume: camera, bind share two");
                mCameraRender.bindShareEnv(mShareTwoRender.getNativePtr());
            }
        } catch (Exception e) {
            LogUtil.i(TAG, "handleCameraRenderResume: exception = " + e.getMessage());
            e.printStackTrace();
        }
    }

    private void handleShareTwoEnvPrepare() {
        try {
            LogUtil.i(TAG, "handleShareTwoEnvPrepare");
            synchronized (mShareTwoSubLock) {
                if (mShareTwoSubRender == null || !mShareTwoSubRender.isInitialized()) {
                    LogUtil.i(TAG, "handleShareTwoEnvPrepare: share two sub wait");
                    mShareTwoSubLock.wait();
                    LogUtil.i(TAG, "handleShareTwoEnvPrepare: share two sub resume");
                }
                mShareTwoRender.bindShareEnv(mShareTwoSubRender.getNativePtr());
            }
        } catch (Exception e) {
            LogUtil.i(TAG, "handleShareTwoEnvPrepare: exception = " + e.getMessage());
            e.printStackTrace();
        }
    }

}
