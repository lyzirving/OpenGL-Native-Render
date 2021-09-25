package com.render.demo.ui;

import android.content.Intent;
import android.graphics.PixelFormat;
import android.view.SurfaceHolder;
import android.view.SurfaceView;
import android.view.View;
import android.widget.SeekBar;

import androidx.annotation.NonNull;

import com.render.demo.R;
import com.render.engine.core.RenderAdapter;
import com.render.engine.filter.ContrastFilter;
import com.render.engine.filter.ExposureFilter;
import com.render.engine.filter.FilterConst;
import com.render.engine.filter.GaussianFilter;
import com.render.engine.filter.HighlightShadowFilter;
import com.render.engine.filter.SaturationFilter;
import com.render.engine.filter.SharpenFilter;
import com.render.engine.img.ImageRenderEngine;
import com.render.engine.util.LogUtil;

/**
 * @author lyzirving
 */
public class StillImageActivity extends BaseActivity implements View.OnClickListener, SurfaceHolder.Callback, SeekBar.OnSeekBarChangeListener {
    private static final String TAG = "StillImageActivity";

    private SurfaceView mSurfaceView;
    private ImageRenderEngine mImgRender;
    private RenderAdapter mRenderAdapter;

    private View mAdjustBeautyRoot, mAdjustBrightnessRoot, mAdjustBlurRoot;
    private SeekBar mContrastSeekBar, mSharpenSeekBar, mSaturationSeekBar;
    private SeekBar mExposureSeekBar, mIncShadowSeekBar, mDecHighlightSeekBar;
    private SeekBar mHorBlurSeekBar, mVerBlurSeekBar;

    private ContrastFilter mContrastFilter;
    private SharpenFilter mSharpenFilter;
    private SaturationFilter mSaturationFilter;

    private ExposureFilter mExposureFilter;
    private HighlightShadowFilter mHighlightShadowFilter;
    private GaussianFilter mGaussianFilter;

    @Override
    protected int getLayoutId() {
        return R.layout.activity_still_image;
    }

    @Override
    protected void initView() {
        mSurfaceView = findViewById(R.id.render_surface);
        mSurfaceView.setZOrderOnTop(false);
        mSurfaceView.getHolder().setFormat(PixelFormat.TRANSPARENT);
        mSurfaceView.getHolder().addCallback(this);

        mAdjustBeautyRoot = findViewById(R.id.layout_adjust_beauty_root);
        mContrastSeekBar = findViewById(R.id.seek_bar_contrast);
        mSharpenSeekBar = findViewById(R.id.seek_bar_sharpen);
        mSaturationSeekBar = findViewById(R.id.seek_bar_saturation);

        mAdjustBrightnessRoot = findViewById(R.id.layout_adjust_brightness_root);
        mExposureSeekBar = findViewById(R.id.seek_bar_exposure);
        mIncShadowSeekBar = findViewById(R.id.seek_bar_inc_shadow);
        mDecHighlightSeekBar = findViewById(R.id.seek_bar_dec_highlight);

        mAdjustBlurRoot = findViewById(R.id.layout_adjust_blur_root);
        mHorBlurSeekBar = findViewById(R.id.seek_bar_hor_blur);
        mVerBlurSeekBar = findViewById(R.id.seek_bar_ver_blur);

        mContrastSeekBar.setOnSeekBarChangeListener(this);
        mSharpenSeekBar.setOnSeekBarChangeListener(this);
        mSaturationSeekBar.setOnSeekBarChangeListener(this);
        mExposureSeekBar.setOnSeekBarChangeListener(this);
        mIncShadowSeekBar.setOnSeekBarChangeListener(this);
        mDecHighlightSeekBar.setOnSeekBarChangeListener(this);
        mHorBlurSeekBar.setOnSeekBarChangeListener(this);
        mVerBlurSeekBar.setOnSeekBarChangeListener(this);
        findViewById(R.id.tab_beauty).setOnClickListener(this);
        findViewById(R.id.tab_brightness).setOnClickListener(this);
        findViewById(R.id.tab_blur).setOnClickListener(this);
        findViewById(R.id.tab_clear).setOnClickListener(this);
    }

    @Override
    protected void initData() {
        initDefaultVal();
    }

    @Override
    protected void release() {
        LogUtil.i(TAG, "release");
        if (mImgRender != null) { mImgRender.release(); }
        mImgRender = null;
        mRenderAdapter = null;
        mSurfaceView.getHolder().removeCallback(this);
    }

    @Override
    protected void onNewIntent(Intent intent) {
        super.onNewIntent(intent);
        LogUtil.i(TAG, "onNewIntent");
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
        hideAllTab();
        if (mImgRender != null) { mImgRender.onPause(); }
    }

    @Override
    protected void onStop() {
        super.onStop();
        LogUtil.i(TAG, "onStop");
    }

    @Override
    public void onClick(View v) {
        switch (v.getId()) {
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
    public void onProgressChanged(SeekBar seekBar, int progress, boolean fromUser) {}

    @Override
    public void onStartTrackingTouch(SeekBar seekBar) {}

    @Override
    public void onStopTrackingTouch(SeekBar seekBar) {
        switch (seekBar.getId()) {
            case R.id.seek_bar_contrast: {
                mContrastFilter.adjust(seekBar.getProgress());
                mImgRender.requestRender();
                break;
            }
            case R.id.seek_bar_sharpen: {
                mSharpenFilter.adjust(seekBar.getProgress());
                mImgRender.requestRender();
                break;
            }
            case R.id.seek_bar_saturation: {
                mSaturationFilter.adjust(seekBar.getProgress());
                mImgRender.requestRender();
                break;
            }
            case R.id.seek_bar_exposure: {
                mExposureFilter.adjust(seekBar.getProgress());
                mImgRender.requestRender();
                break;
            }
            case R.id.seek_bar_inc_shadow: {
                mHighlightShadowFilter.adjustProperty(FilterConst.SHADOW, seekBar.getProgress());
                mImgRender.requestRender();
                break;
            }
            case R.id.seek_bar_dec_highlight: {
                mHighlightShadowFilter.adjustProperty(FilterConst.HIGHLIGHT, seekBar.getProgress());
                mImgRender.requestRender();
                break;
            }
            case R.id.seek_bar_hor_blur: {
                mGaussianFilter.adjustProperty(FilterConst.HOR_GAUSSIAN, seekBar.getProgress());
                mImgRender.requestRender();
                break;
            }
            case R.id.seek_bar_ver_blur: {
                mGaussianFilter.adjustProperty(FilterConst.VER_GAUSSIAN, seekBar.getProgress());
                mImgRender.requestRender();
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
        if (mImgRender == null || !mImgRender.isInitialized()) {
            mImgRender = new ImageRenderEngine();
            mImgRender.onSurfaceCreate(holder.getSurface(), getRenderAdapter());
        } else {
            mImgRender.onResume(holder.getSurface());
        }
    }

    @Override
    public void surfaceChanged(@NonNull SurfaceHolder holder, int format, int width, int height) {
        LogUtil.i(TAG, "surfaceChanged: width = " + width + ", height = " + height);
        if (mImgRender != null) {
            mImgRender.onSurfaceChange(width, height);
            mImgRender.setResource(getApplicationContext(), R.drawable.lenna);
            //we should explicitly call requestRender() when surface size is changed
            mImgRender.requestRender();
        }
    }

    @Override
    public void surfaceDestroyed(@NonNull SurfaceHolder holder) {
        LogUtil.i(TAG, "surfaceDestroyed");
        //don't release Render here, we should release render when we do not need it, such as in onDestroy()
    }

    private boolean anyTabShow() {
        return mAdjustBeautyRoot.getVisibility() == View.VISIBLE || mAdjustBrightnessRoot.getVisibility() == View.VISIBLE
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
            };
        }
        return mRenderAdapter;
    }

    private void hideAllTab() {
        mAdjustBeautyRoot.setVisibility(View.GONE);
        mAdjustBrightnessRoot.setVisibility(View.GONE);
        mAdjustBlurRoot.setVisibility(View.GONE);
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
            mImgRender.addBeautyFilter(mContrastFilter, false);
            mImgRender.addBeautyFilter(mSharpenFilter, false);
            mImgRender.addBeautyFilter(mSaturationFilter, true);
            mImgRender.requestRender();
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
            mImgRender.addBeautyFilter(mExposureFilter, false);
            mImgRender.addBeautyFilter(mHighlightShadowFilter, true);
            mImgRender.requestRender();
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
            mImgRender.addBeautyFilter(mGaussianFilter, true);
            mImgRender.requestRender();
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
        //brightness related
        mExposureFilter = null;
        mHighlightShadowFilter = null;

        mGaussianFilter = null;

        mImgRender.clearBeautyFilter();
        mImgRender.requestRender();
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
