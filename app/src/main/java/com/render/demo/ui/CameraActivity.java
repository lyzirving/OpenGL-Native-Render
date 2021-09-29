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
import android.widget.CompoundButton;
import android.widget.SeekBar;
import android.widget.Switch;

import androidx.annotation.NonNull;

import com.render.demo.R;
import com.render.engine.camera.CameraHelper;
import com.render.engine.camera.RenderCamMetadata;
import com.render.engine.camera.CamRenderEngine;
import com.render.engine.core.RenderAdapter;
import com.render.engine.filter.ContrastFilter;
import com.render.engine.filter.ExposureFilter;
import com.render.engine.filter.FilterConst;
import com.render.engine.filter.GaussianFilter;
import com.render.engine.filter.HighlightShadowFilter;
import com.render.engine.filter.SaturationFilter;
import com.render.engine.filter.SharpenFilter;
import com.render.engine.util.LogUtil;

/**
 * @author lyzirving
 */
public class CameraActivity extends BaseActivity implements SurfaceHolder.Callback, View.OnClickListener,
        SeekBar.OnSeekBarChangeListener, CompoundButton.OnCheckedChangeListener {
    private static final String TAG = "CameraActivity";

    private SurfaceView mSurface;
    private CamRenderEngine mCameraRender;
    private RenderAdapter mRenderAdapter;
    private int mSurfaceWidth, mSurfaceHeight;

    private View mAdjustBeautyRoot, mAdjustBrightnessRoot, mAdjustBlurRoot;
    private SeekBar mContrastSeekBar, mSharpenSeekBar, mSaturationSeekBar;
    private SeekBar mExposureSeekBar, mIncShadowSeekBar, mDecHighlightSeekBar;
    private SeekBar mHorBlurSeekBar, mVerBlurSeekBar;

    private ContrastFilter mContrastFilter;
    private SaturationFilter mSaturationFilter;
    private SharpenFilter mSharpenFilter;
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
        findViewById(R.id.tab_beauty).setOnClickListener(this);
        findViewById(R.id.tab_brightness).setOnClickListener(this);
        findViewById(R.id.tab_blur).setOnClickListener(this);
        findViewById(R.id.tab_clear).setOnClickListener(this);

        ((Switch)(findViewById(R.id.switch_detect_face))).setOnCheckedChangeListener(this);

        mAdjustBeautyRoot = findViewById(R.id.layout_adjust_beauty_root);
        mAdjustBrightnessRoot = findViewById(R.id.layout_adjust_brightness_root);
        mAdjustBlurRoot = findViewById(R.id.layout_adjust_blur_root);

        mContrastSeekBar = findViewById(R.id.seek_bar_contrast);
        mSharpenSeekBar = findViewById(R.id.seek_bar_sharpen);
        mSaturationSeekBar = findViewById(R.id.seek_bar_saturation);

        mContrastSeekBar.setOnSeekBarChangeListener(this);
        mSharpenSeekBar.setOnSeekBarChangeListener(this);
        mSaturationSeekBar.setOnSeekBarChangeListener(this);

        mExposureSeekBar = findViewById(R.id.seek_bar_exposure);
        mIncShadowSeekBar = findViewById(R.id.seek_bar_inc_shadow);
        mDecHighlightSeekBar = findViewById(R.id.seek_bar_dec_highlight);

        mExposureSeekBar.setOnSeekBarChangeListener(this);
        mIncShadowSeekBar.setOnSeekBarChangeListener(this);
        mDecHighlightSeekBar.setOnSeekBarChangeListener(this);

        mHorBlurSeekBar = findViewById(R.id.seek_bar_hor_blur);
        mVerBlurSeekBar = findViewById(R.id.seek_bar_ver_blur);

        mHorBlurSeekBar.setOnSeekBarChangeListener(this);
        mVerBlurSeekBar.setOnSeekBarChangeListener(this);
    }

    @Override
    protected void initData() {
        initDefaultVal();
    }

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
            case R.id.tab_brightness: {
                handleClickBrightness();
                break;
            }
            case R.id.tab_blur: {
                handleClickBlur();
                break;
            }
            case R.id.tab_clear: {
                handleClickClear();
                break;
            }
            default: {
                break;
            }
        }
    }

    @Override
    public void onCheckedChanged(CompoundButton buttonView, boolean isChecked) {
        switch (buttonView.getId()) {
            case R.id.switch_detect_face: {
                mCameraRender.detect(isChecked);
                break;
            }
            default: {
                break;
            }
        }
    }

    @Override
    public void onProgressChanged(SeekBar seekBar, int progress, boolean fromUser) {}

    @Override
    public void onStartTrackingTouch(SeekBar seekBar) {}

    @Override
    public void onStopTrackingTouch(SeekBar seekBar) {
        switch (seekBar.getId()) {
            case R.id.seek_bar_contrast: {
                mContrastFilter.adjust(seekBar.getProgress());
                mCameraRender.requestRender();
                break;
            }
            case R.id.seek_bar_sharpen: {
                mSharpenFilter.adjust(seekBar.getProgress());
                mCameraRender.requestRender();
                break;
            }
            case R.id.seek_bar_saturation: {
                mSaturationFilter.adjust(seekBar.getProgress());
                mCameraRender.requestRender();
                break;
            }
            case R.id.seek_bar_exposure: {
                mExposureFilter.adjust(seekBar.getProgress());
                mCameraRender.requestRender();
                break;
            }
            case R.id.seek_bar_inc_shadow: {
                mHighlightShadowFilter.adjustProperty(FilterConst.SHADOW, seekBar.getProgress());
                mCameraRender.requestRender();
                break;
            }
            case R.id.seek_bar_dec_highlight: {
                mHighlightShadowFilter.adjustProperty(FilterConst.HIGHLIGHT, seekBar.getProgress());
                mCameraRender.requestRender();
                break;
            }
            case R.id.seek_bar_hor_blur: {
                mGaussianFilter.adjustProperty(FilterConst.HOR_GAUSSIAN, seekBar.getProgress());
                mCameraRender.requestRender();
                break;
            }
            case R.id.seek_bar_ver_blur: {
                mGaussianFilter.adjustProperty(FilterConst.VER_GAUSSIAN, seekBar.getProgress());
                mCameraRender.requestRender();
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
        LogUtil.i(TAG, "surfaceDestroyed");
    }

    private boolean anyTabShow() {
        return mAdjustBeautyRoot.getVisibility() == View.VISIBLE
                || mAdjustBrightnessRoot.getVisibility() == View.VISIBLE
                || mAdjustBlurRoot.getVisibility() == View.VISIBLE;
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
        mAdjustBrightnessRoot.setVisibility(View.GONE);
        mAdjustBlurRoot.setVisibility(View.GONE);
    }

    private void handleClickSwitchCamera() {
        if (CameraHelper.get().isCameraOpen()) {
            mCameraRender.onPause();
            CameraHelper.get().closeCameraAndWait();
            CameraHelper.get().switchFrontType();
            mCameraRender.onResume(mSurface.getHolder().getSurface());
            CameraHelper.get().prepare(getApplicationContext(), mSurfaceWidth, mSurfaceHeight);
            Size previewSize = CameraHelper.get().getPreviewSize();
            RenderCamMetadata data = new RenderCamMetadata.Builder()
                    .previewSize(previewSize.getWidth(), previewSize.getHeight())
                    .frontType(CameraHelper.get().getFrontType()).build();
            mCameraRender.setRenderCamMetadata(data);
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
        if (!show && mContrastFilter == null) {
            mContrastFilter = new ContrastFilter();
            mSharpenFilter = new SharpenFilter();
            mSaturationFilter = new SaturationFilter();
            mCameraRender.addBeautyFilter(mContrastFilter, false);
            mCameraRender.addBeautyFilter(mSaturationFilter, false);
            mCameraRender.addBeautyFilter(mSharpenFilter, true);
            mCameraRender.requestRender();
        }
    }

    private void handleClickBrightness() {
        if (anyTabShow()) {
            hideAllTab();
            return;
        }
        boolean show = mAdjustBrightnessRoot.getVisibility() == View.VISIBLE;
        showTab(mAdjustBrightnessRoot, !show);
        if (!show && mExposureFilter == null) {
            mExposureFilter = new ExposureFilter();
            mHighlightShadowFilter = new HighlightShadowFilter();
            mCameraRender.addBeautyFilter(mExposureFilter, false);
            mCameraRender.addBeautyFilter(mHighlightShadowFilter, true);
            mCameraRender.requestRender();
        }
    }

    private void handleClickBlur() {
        if (anyTabShow()) {
            hideAllTab();
            return;
        }
        boolean show = mAdjustBlurRoot.getVisibility() == View.VISIBLE;
        showTab(mAdjustBlurRoot, !show);
        if (!show && mGaussianFilter == null) {
            mGaussianFilter = new GaussianFilter();
            mCameraRender.addBeautyFilter(mGaussianFilter, true);
            mCameraRender.requestRender();
        }
    }

    private void handleClickClear() {
        if (anyTabShow()) {
            hideAllTab();
        }
        initDefaultVal();
        //beauty filter related
        mContrastFilter = null;
        mSharpenFilter = null;
        mSaturationFilter = null;
        mExposureFilter = null;
        mHighlightShadowFilter = null;
        mGaussianFilter = null;

        mCameraRender.clearBeautyFilter();
        mCameraRender.requestRender();
    }

    private void handleOesTextureCreate(int oesTexture) {
        SurfaceTexture surfaceTexture = new SurfaceTexture(oesTexture);
        mCameraRender.setSurfaceTexture(surfaceTexture);
        CameraHelper.get().setOesTexture(surfaceTexture);
        CameraHelper.get().setOnFrameAvailableListener(mCameraRender);
        CameraHelper.get().open(getApplicationContext());
    }

    private void initDefaultVal() {
        if (mContrastSeekBar != null) { mContrastSeekBar.setProgress(50); }
        if (mSharpenSeekBar != null) { mSharpenSeekBar.setProgress(50); }
        if (mSaturationSeekBar != null) { mSaturationSeekBar.setProgress(50); }
        if (mExposureSeekBar != null) { mExposureSeekBar.setProgress(0); }
        if (mIncShadowSeekBar != null) { mIncShadowSeekBar.setProgress(0); }
        if (mDecHighlightSeekBar != null) { mDecHighlightSeekBar.setProgress(0); }
        if (mHorBlurSeekBar != null) { mHorBlurSeekBar.setProgress(0); }
        if (mVerBlurSeekBar != null) { mVerBlurSeekBar.setProgress(0); }
    }

    private void showTab(View view, boolean show) {
        view.setVisibility(show ? View.VISIBLE : View.GONE);
    }
}
