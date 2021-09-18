package com.render.demo.ui;

import android.annotation.SuppressLint;
import android.graphics.PixelFormat;
import android.graphics.SurfaceTexture;
import android.os.Handler;
import android.os.Looper;
import android.os.Message;
import android.util.Size;
import android.view.SurfaceHolder;
import android.view.SurfaceView;
import android.widget.FrameLayout;

import androidx.annotation.NonNull;

import com.render.demo.R;
import com.render.engine.camera.CameraHelper;
import com.render.engine.camera.RenderCamMetadata;
import com.render.engine.core.CameraRenderEngine;
import com.render.engine.core.RenderAdapter;
import com.render.engine.util.LogUtil;

public class CameraActivity extends BaseActivity implements SurfaceHolder.Callback {
    private static final String TAG = "CameraActivity";

    private FrameLayout mFlCameraContainer;
    private SurfaceView mSurfaceView;
    private CameraRenderEngine mCameraRender;
    private RenderAdapter mRenderAdapter;

    private static final int MSG_RENDER_PREPARE = 1;
    private Handler mMainHandler = new Handler(Looper.getMainLooper()) {
        @Override
        public void handleMessage(@NonNull Message msg) {
            super.handleMessage(msg);
            switch (msg.what) {
                case MSG_RENDER_PREPARE: {
                    handleRenderEnvPrepare(msg.arg1);
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
        return R.layout.activity_camera;
    }

    @Override
    protected void initView() {
        mFlCameraContainer = findViewById(R.id.fl_camera_container);
    }

    @Override
    protected void initData() {
        CameraHelper.get().prepare(getApplicationContext());
        mFlCameraContainer.post(()->{
            mSurfaceView = new SurfaceView(getApplicationContext());
            Size size = CameraHelper.get().getPreviewSize();
            float ratio = size.getHeight() * 1f / size.getWidth();
            int surfaceHeight = (int)(mFlCameraContainer.getMeasuredWidth() * ratio);
            FrameLayout.LayoutParams lp = new FrameLayout.LayoutParams(mFlCameraContainer.getMeasuredWidth(), surfaceHeight);
            mFlCameraContainer.addView(mSurfaceView, lp);
            mSurfaceView.setZOrderOnTop(false);
            mSurfaceView.getHolder().setFormat(PixelFormat.TRANSPARENT);
            mSurfaceView.getHolder().addCallback(this);
        });
    }

    @Override
    protected void release() {
        LogUtil.i(TAG, "release");
        if (mCameraRender != null) { mCameraRender.release(); }
        mCameraRender = null;
    }

    @Override
    protected void onResume() {
        super.onResume();
        LogUtil.i(TAG, "onResume");
    }

    @Override
    protected void onPause() {
        super.onPause();
        LogUtil.i(TAG, "onPause");
        CameraHelper.get().closeCamera();
        if (mCameraRender != null) { mCameraRender.onPause(); }
    }

    @Override
    protected void onStop() {
        super.onStop();
        LogUtil.i(TAG, "onStop");
    }

    private RenderAdapter getRenderAdapter() {
        if (mRenderAdapter == null) {
            mRenderAdapter = new RenderAdapter() {
                @Override
                public void onRenderEnvPrepare(int textureId) {
                    super.onRenderEnvPrepare(textureId);
                    LogUtil.i(TAG, "onRenderEnvPrepare: " + textureId);
                    mMainHandler.obtainMessage(MSG_RENDER_PREPARE, textureId, 0).sendToTarget();
                }

                @Override
                public void onRenderEnvRelease() {
                    super.onRenderEnvRelease();
                    LogUtil.i(TAG, "onRenderEnvRelease");
                }
            };
        }
        return mRenderAdapter;
    }

    @Override
    public void surfaceCreated(@NonNull SurfaceHolder holder) {
        LogUtil.i(TAG, "surfaceCreated");
        if (mCameraRender == null || !mCameraRender.isInitialized()) {
            mCameraRender = new CameraRenderEngine();
            mCameraRender.onSurfaceCreate(holder.getSurface(), getRenderAdapter());
        } else {
            mCameraRender.onResume(holder.getSurface());
        }
    }

    @Override
    public void surfaceChanged(@NonNull SurfaceHolder holder, int format, int width, int height) {
        LogUtil.i(TAG, "surfaceChanged: width = " + width + ", height = " + height);
        if (mCameraRender != null) {
            mCameraRender.onSurfaceChange(width, height);
        }
    }

    @Override
    public void surfaceDestroyed(@NonNull SurfaceHolder holder) {
        LogUtil.i(TAG, "surfaceDestroyed");
    }

    private void handleRenderEnvPrepare(int textureId) {
        if (!CameraHelper.get().isPrepared()) { CameraHelper.get().prepare(getApplicationContext()); }
        @SuppressLint("Recycle")
        SurfaceTexture surfaceTexture = new SurfaceTexture(textureId);
        CameraHelper.get().setOesTexture(surfaceTexture);
        mCameraRender.setSurfaceTexture(surfaceTexture);
        RenderCamMetadata data = new RenderCamMetadata.Builder()
                .previewSize(CameraHelper.get().getPreviewSize().getWidth(), CameraHelper.get().getPreviewSize().getHeight())
                .frontType(CameraHelper.get().getFrontType()).build();
        mCameraRender.setRenderCamMetadata(data);
        CameraHelper.get().setOnFrameAvailableListener(mCameraRender);
        CameraHelper.get().open(getApplicationContext());
    }
}
