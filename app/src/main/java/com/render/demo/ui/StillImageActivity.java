package com.render.demo.ui;

import android.content.Intent;
import android.graphics.PixelFormat;
import android.os.Handler;
import android.os.Looper;
import android.os.Message;
import android.view.SurfaceHolder;
import android.view.SurfaceView;
import android.view.View;
import android.widget.CompoundButton;
import android.widget.SeekBar;
import android.widget.Switch;

import androidx.annotation.NonNull;

import com.airbnb.lottie.LottieAnimationView;
import com.render.demo.R;
import com.render.engine.core.RenderAdapter;
import com.render.engine.filter.ColorAdjustFilter;
import com.render.engine.filter.ExposureFilter;
import com.render.engine.filter.FilterConst;
import com.render.engine.filter.GaussianFilter;
import com.render.engine.filter.HighlightShadowFilter;
import com.render.engine.img.ImageRenderEngine;
import com.render.engine.util.LogUtil;

/**
 * @author lyzirving
 */
public class StillImageActivity extends BaseActivity implements View.OnClickListener, SurfaceHolder.Callback,
        SeekBar.OnSeekBarChangeListener, CompoundButton.OnCheckedChangeListener {
    private static final String TAG = "StillImageActivity";

    private SurfaceView mSurfaceView;
    private ImageRenderEngine mImgRender;
    private RenderAdapter mRenderAdapter;

    private View mAdjustBeautyRoot, mAdjustBrightnessRoot, mAdjustBlurRoot, mAdjustFaceRoot;
    private SeekBar mContrastSeekBar, mSharpenSeekBar, mSaturationSeekBar;
    private SeekBar mExposureSeekBar, mIncShadowSeekBar, mDecHighlightSeekBar;
    private SeekBar mHorBlurSeekBar, mVerBlurSeekBar;
    private SeekBar mFaceLiftSeekBar;
    private SeekBar mFaceBeautifySeekBar, mSkinBuffSeekBar;

    private ColorAdjustFilter mColorAdjustFilter;
    private ExposureFilter mExposureFilter;
    private HighlightShadowFilter mHighlightShadowFilter;
    private GaussianFilter mGaussianFilter;

    private Switch mSwitchDetectFace, mSwitchBeautifyFace;
    private LottieAnimationView mLottieLoading;

    private static final int MSG_SHOW_LOADING_ANI = 1;
    private Handler mMainHandler = new Handler(Looper.getMainLooper()) {
        @Override
        public void handleMessage(@NonNull Message msg) {
            super.handleMessage(msg);
            switch (msg.what) {
                case MSG_SHOW_LOADING_ANI: {
                    showLoadingView(msg.arg1 > 0);
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
        return R.layout.activity_still_image;
    }

    @Override
    protected void initView() {
        mSurfaceView = findViewById(R.id.render_surface);
        mSurfaceView.setZOrderOnTop(false);
        mSurfaceView.getHolder().setFormat(PixelFormat.TRANSPARENT);
        mSurfaceView.getHolder().addCallback(this);

        mLottieLoading = findViewById(R.id.lottie_loading);

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

        mAdjustFaceRoot = findViewById(R.id.layout_adjust_face_root);
        mFaceLiftSeekBar = findViewById(R.id.seek_bar_face_lift_intensity);

        mFaceBeautifySeekBar = findViewById(R.id.seek_bar_face_beautify_intensity);
        mSkinBuffSeekBar = findViewById(R.id.seek_bar_skin_buff_intensity);

        mSwitchDetectFace = findViewById(R.id.switch_detect_face);
        mSwitchBeautifyFace = findViewById(R.id.switch_beautify_face);
        mSwitchDetectFace.setOnCheckedChangeListener(this);
        mSwitchBeautifyFace.setOnCheckedChangeListener(this);

        mContrastSeekBar.setOnSeekBarChangeListener(this);
        mSharpenSeekBar.setOnSeekBarChangeListener(this);
        mSaturationSeekBar.setOnSeekBarChangeListener(this);
        mExposureSeekBar.setOnSeekBarChangeListener(this);
        mIncShadowSeekBar.setOnSeekBarChangeListener(this);
        mDecHighlightSeekBar.setOnSeekBarChangeListener(this);
        mHorBlurSeekBar.setOnSeekBarChangeListener(this);
        mVerBlurSeekBar.setOnSeekBarChangeListener(this);
        mFaceLiftSeekBar.setOnSeekBarChangeListener(this);
        mFaceBeautifySeekBar.setOnSeekBarChangeListener(this);
        mSkinBuffSeekBar.setOnSeekBarChangeListener(this);
        findViewById(R.id.tab_beauty).setOnClickListener(this);
        findViewById(R.id.tab_brightness).setOnClickListener(this);
        findViewById(R.id.tab_blur).setOnClickListener(this);
        findViewById(R.id.tab_face).setOnClickListener(this);
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
            case R.id.tab_face: {
                handleClickFace();
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
                mImgRender.trackFace(isChecked);
                break;
            }
            case R.id.switch_beautify_face: {
                mImgRender.beautifyFace(isChecked);
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
                mColorAdjustFilter.adjustProperty(FilterConst.PROP_CONTRAST, seekBar.getProgress());
                mImgRender.requestRender();
                break;
            }
            case R.id.seek_bar_sharpen: {
                mColorAdjustFilter.adjustProperty(FilterConst.PROP_SHARPEN, seekBar.getProgress());
                mImgRender.requestRender();
                break;
            }
            case R.id.seek_bar_saturation: {
                mColorAdjustFilter.adjustProperty(FilterConst.PROP_SATURATION, seekBar.getProgress());
                mImgRender.requestRender();
                break;
            }
            case R.id.seek_bar_exposure: {
                mExposureFilter.adjust(seekBar.getProgress());
                mImgRender.requestRender();
                break;
            }
            case R.id.seek_bar_inc_shadow: {
                mHighlightShadowFilter.adjustProperty(FilterConst.PROP_SHADOW, seekBar.getProgress());
                mImgRender.requestRender();
                break;
            }
            case R.id.seek_bar_dec_highlight: {
                mHighlightShadowFilter.adjustProperty(FilterConst.PROP_HIGHLIGHT, seekBar.getProgress());
                mImgRender.requestRender();
                break;
            }
            case R.id.seek_bar_hor_blur: {
                mGaussianFilter.adjustProperty(FilterConst.PROP_HOR_GAUSSIAN, seekBar.getProgress());
                mImgRender.requestRender();
                break;
            }
            case R.id.seek_bar_ver_blur: {
                mGaussianFilter.adjustProperty(FilterConst.PROP_VER_GAUSSIAN, seekBar.getProgress());
                mImgRender.requestRender();
                break;
            }
            case R.id.seek_bar_face_lift_intensity: {
                mImgRender.adjustProperty(FilterConst.FACE_LIFT, FilterConst.PROP_FACE_LIFT_INTENSITY, seekBar.getProgress());
                mImgRender.requestRender();
                break;
            }
            case R.id.seek_bar_face_beautify_intensity: {
                mImgRender.adjustProperty(FilterConst.BEAUTIFY_FACE, FilterConst.PROP_BEAUTIFY_SKIN, seekBar.getProgress());
                mImgRender.requestRender();
                break;
            }
            case R.id.seek_bar_skin_buff_intensity: {
                mImgRender.adjustProperty(FilterConst.BEAUTIFY_FACE, FilterConst.PROP_SKIN_BUFF, seekBar.getProgress());
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
            mImgRender.setResource(getApplicationContext(), R.drawable.test_portrait);
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

                @Override
                public void onTrackImageLandMarkStart() {
                    super.onTrackImageLandMarkStart();
                    LogUtil.i(TAG, "onTrackImageLandMarkStart");
                    mMainHandler.obtainMessage(MSG_SHOW_LOADING_ANI, 1, 0).sendToTarget();
                }

                @Override
                public void onTrackImageLandMarkFinish() {
                    super.onTrackImageLandMarkFinish();
                    LogUtil.i(TAG, "onTrackImageLandMarkFinish");
                    mMainHandler.obtainMessage(MSG_SHOW_LOADING_ANI, 0, 0).sendToTarget();
                }
            };
        }
        return mRenderAdapter;
    }

    private void hideAllTab() {
        mAdjustBeautyRoot.setVisibility(View.GONE);
        mAdjustBrightnessRoot.setVisibility(View.GONE);
        mAdjustBlurRoot.setVisibility(View.GONE);
        mAdjustFaceRoot.setVisibility(View.GONE);
    }

    private void handleClickBeauty() {
        if (anyTabShow()) {
            hideAllTab();
        } else {
            boolean show = mAdjustBeautyRoot.getVisibility() == View.VISIBLE;
            showTab(mAdjustBeautyRoot, !show);
            if (!show && mColorAdjustFilter == null) {
                mColorAdjustFilter = new ColorAdjustFilter();
                mImgRender.addBeautyFilter(mColorAdjustFilter, true);
                mImgRender.requestRender();
            }
        }
    }

    private void handleClickBrightness() {
        if (anyTabShow()) {
            hideAllTab();
        } else {
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
    }

    private void handleClickBlur() {
        if (anyTabShow()) {
            hideAllTab();
        } else {
            boolean show = mAdjustBlurRoot.getVisibility() == View.VISIBLE;
            showTab(mAdjustBlurRoot, !show);
            if (!show && mGaussianFilter == null) {
                mGaussianFilter = new GaussianFilter();
                mImgRender.addBeautyFilter(mGaussianFilter, true);
                mImgRender.requestRender();
            }
        }
    }

    private void handleClickFace() {
        if (anyTabShow()) {
            hideAllTab();
        } else {
            boolean show = mAdjustFaceRoot.getVisibility() == View.VISIBLE;
            showTab(mAdjustFaceRoot, !show);
            if (!show && !mSwitchDetectFace.isChecked()) {
                mSwitchDetectFace.setChecked(true);
            }
        }
    }

    private void handleClickClear() {
        if (anyTabShow()) {
            hideAllTab();
        }
        initDefaultVal();
        mExposureFilter = null;
        mHighlightShadowFilter = null;
        mGaussianFilter = null;
        mColorAdjustFilter = null;

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
        if (mFaceLiftSeekBar != null) { mFaceLiftSeekBar.setProgress(50); }
        if (mSkinBuffSeekBar != null) { mSkinBuffSeekBar.setProgress(50); }
        if (mFaceBeautifySeekBar != null) { mFaceBeautifySeekBar.setProgress(50); }
    }

    private void showLoadingView(boolean show) {
        mLottieLoading.setVisibility(show ? View.VISIBLE : View.GONE);
        if (show) {
            mLottieLoading.playAnimation();
        } else {
            mLottieLoading.pauseAnimation();
        }
    }

    private void showTab(View view, boolean show) {
        view.setVisibility(show ? View.VISIBLE : View.GONE);
    }
}
