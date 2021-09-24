package com.render.demo.ui;

import android.graphics.PixelFormat;
import android.graphics.SurfaceTexture;
import android.os.Handler;
import android.os.Looper;
import android.os.Message;
import android.util.Size;
import android.view.SurfaceHolder;
import android.view.SurfaceView;
import android.view.View;

import androidx.annotation.NonNull;

import com.render.demo.R;
import com.render.engine.camera.CameraHelper;
import com.render.engine.camera.RenderCamMetadata;
import com.render.engine.core.CamRenderEngine;
import com.render.engine.core.RenderAdapter;
import com.render.engine.core.filter.ContrastFilter;
import com.render.engine.core.filter.ExposureFilter;
import com.render.engine.core.filter.GaussianFilter;
import com.render.engine.core.filter.HighlightShadowFilter;
import com.render.engine.core.filter.SaturationFilter;
import com.render.engine.core.filter.SharpenFilter;
import com.render.engine.util.LogUtil;

/**
 * @author lyzirving
 */
public class CameraActivity extends BaseActivity implements SurfaceHolder.Callback, View.OnClickListener {
    private static final String TAG = "CameraActivity";

    private SurfaceView mSurface;
    private CamRenderEngine mCameraRender;
    private RenderAdapter mRenderAdapter;
    private int mSurfaceWidth, mSurfaceHeight;

    private View mAdjustBeautyRoot;

    private ContrastFilter mContrastFilter;
    private SharpenFilter mSharpenFilter;
    private SaturationFilter mSaturationFilter;
    private ExposureFilter mExposureFilter;
    private HighlightShadowFilter mHighlightShadowFilter;
    private GaussianFilter mGaussianFilter;

    private static final int MSG_RENDER_PREPARE = 1;
    private static final int MSG_RENDER_OES_TEXTURE_CREATE = 2;
    private Handler mMainHandler = new Handler(Looper.getMainLooper()) {
        @Override
        public void handleMessage(@NonNull Message msg) {
            super.handleMessage(msg);
            switch (msg.what) {
                case MSG_RENDER_OES_TEXTURE_CREATE: {
                    handleOesTextureCreate(msg.arg1);
                    break;
                }
                case MSG_RENDER_PREPARE:
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
        mSurface = findViewById(R.id.camera_view);
        mSurface.setZOrderOnTop(false);
        mSurface.getHolder().setFormat(PixelFormat.TRANSPARENT);
        mSurface.getHolder().addCallback(this);

        findViewById(R.id.tab_switch_camera).setOnClickListener(this);

        mAdjustBeautyRoot = findViewById(R.id.layout_adjust_beauty_root);
        findViewById(R.id.tab_beauty).setOnClickListener(this);
    }

    @Override
    protected void initData() {}

    @Override
    protected void release() {
        LogUtil.i(TAG, "release");
        CameraHelper.get().release();
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

    @Override
    public void onClick(View v) {
        switch (v.getId()) {
            case R.id.tab_switch_camera: {
                handleClickSwitchCamera();
                break;
            }
            case R.id.tab_beauty: {
                handleClickBeauty();
                break;
            }
            default: {
                break;
            }
        }
    }

    @Override
    public void surfaceCreated(@NonNull SurfaceHolder holder) {
        LogUtil.i(TAG, "surfaceCreated");
        if (mCameraRender == null || !mCameraRender.isInitialized()) {
            mCameraRender = new CamRenderEngine();
            mCameraRender.onSurfaceCreate(holder.getSurface(), getRenderAdapter());
        } else {
            mCameraRender.onResume(holder.getSurface());
        }
    }

    @Override
    public void surfaceChanged(@NonNull SurfaceHolder holder, int format, int width, int height) {
        LogUtil.i(TAG, "surfaceChanged: width = " + width + ", height = " + height);
        mSurfaceWidth = width;
        mSurfaceHeight = height;
        if (mCameraRender != null) {
            CameraHelper.get().prepare(getApplicationContext(), width, height);
            Size previewSize = CameraHelper.get().getPreviewSize();
            //the method must be called by order onPreviewChange() -> onSurfaceChange() -> buildTexture()
            mCameraRender.onPreviewChange(previewSize.getWidth(), previewSize.getHeight());
            mCameraRender.onSurfaceChange(width, height);
            mCameraRender.buildTexture();
        }
    }

    @Override
    public void surfaceDestroyed(@NonNull SurfaceHolder holder) {
        LogUtil.i(TAG, "surfaceDestroyed");
    }

    private boolean anyTabShow() {
        return mAdjustBeautyRoot.getVisibility() == View.VISIBLE;
    }

    private RenderAdapter getRenderAdapter() {
        if (mRenderAdapter == null) {
            mRenderAdapter = new RenderAdapter() {
                @Override
                public void onRenderEnvPrepare() {
                    super.onRenderEnvPrepare();
                    LogUtil.i(TAG, "onRenderEnvPrepare");
                }

                @Override
                public void onRenderEnvRelease() {
                    super.onRenderEnvRelease();
                    LogUtil.i(TAG, "onRenderEnvRelease");
                }

                @Override
                public void onRenderOesTextureCreate(int oesTexture) {
                    super.onRenderOesTextureCreate(oesTexture);
                    LogUtil.i(TAG, "onRenderOesTextureCreate: " + oesTexture);
                    mMainHandler.obtainMessage(MSG_RENDER_OES_TEXTURE_CREATE, oesTexture, 0).sendToTarget();
                }
            };
        }
        return mRenderAdapter;
    }

    private void hideAllTab() {
        mAdjustBeautyRoot.setVisibility(View.GONE);
    }

    private void handleClickSwitchCamera() {
        if (CameraHelper.get().isCameraOpen()) {
            mCameraRender.onPause();
            CameraHelper.get().closeCameraAndWait();
            CameraHelper.get().switchFrontType();
            mCameraRender.onResume(mSurface.getHolder().getSurface());
            CameraHelper.get().prepare(getApplicationContext(), mSurfaceWidth, mSurfaceHeight);
            Size previewSize = CameraHelper.get().getPreviewSize();
            //the method must be called by order onPreviewChange() -> onSurfaceChange() -> buildTexture()
            mCameraRender.onPreviewChange(previewSize.getWidth(), previewSize.getHeight());
            mCameraRender.onSurfaceChange(mSurfaceWidth, mSurfaceHeight);
            mCameraRender.buildTexture();
        }
    }

    private void handleClickBeauty() {
        if (anyTabShow()) {
            hideAllTab();
            return;
        }
        boolean show = mAdjustBeautyRoot.getVisibility() == View.VISIBLE;
        showTab(mAdjustBeautyRoot, !show);
        /*if (!show && (mContrastFilter == null || !mContrastFilter.filterValid())) {
            mContrastFilter = new ContrastFilter();
            mSharpenFilter = new SharpenFilter();
            mSaturationFilter = new SaturationFilter();
            mCameraRender.requestRender();
        }*/
    }

    private void handleOesTextureCreate(int oesTexture) {
        SurfaceTexture surfaceTexture = new SurfaceTexture(oesTexture);
        CameraHelper.get().setOesTexture(surfaceTexture);
        mCameraRender.setSurfaceTexture(surfaceTexture);
        RenderCamMetadata data = new RenderCamMetadata.Builder()
                .previewSize(CameraHelper.get().getPreviewSize().getWidth(), CameraHelper.get().getPreviewSize().getHeight())
                .frontType(CameraHelper.get().getFrontType()).build();
        mCameraRender.setRenderCamMetadata(data);
        CameraHelper.get().setOnFrameAvailableListener(mCameraRender);
        CameraHelper.get().open(getApplicationContext());
    }

    private void showTab(View view, boolean show) {
        view.setVisibility(show ? View.VISIBLE : View.GONE);
    }
}
