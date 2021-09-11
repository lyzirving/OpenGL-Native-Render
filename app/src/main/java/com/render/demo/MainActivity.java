package com.render.demo;

import androidx.annotation.NonNull;
import androidx.appcompat.app.AppCompatActivity;

import android.graphics.PixelFormat;
import android.os.Bundle;
import android.view.SurfaceHolder;
import android.view.SurfaceView;
import android.view.View;
import android.widget.SeekBar;

import com.render.engine.core.EngineEnv;
import com.render.engine.core.RenderAdapter;
import com.render.engine.core.RenderEngine;
import com.render.engine.core.filter.ContrastFilter;
import com.render.engine.util.LogUtil;

/**
 * @author lyzirving
 */
public class MainActivity extends AppCompatActivity implements View.OnClickListener, SurfaceHolder.Callback, SeekBar.OnSeekBarChangeListener {
    private static final String TAG = "MainActivity";

    private SurfaceView mSurfaceView;
    private RenderEngine mRender;
    private RenderAdapter mRenderAdapter;

    private View mAdjustBeautyRoot;
    private SeekBar mContrastSeekBar;

    private ContrastFilter mContrastFilter;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);
        initView();
        initData();
    }

    @Override
    protected void onPause() {
        super.onPause();
        LogUtil.i(TAG, "onPause");
        hideAllTab();
        if (mRender != null) { mRender.onPause(); }
    }

    @Override
    protected void onDestroy() {
        super.onDestroy();
        if (mRender != null) { mRender.release(); }
        mRender = null;
        mRenderAdapter = null;
        mSurfaceView.getHolder().removeCallback(this);
    }

    @Override
    public void onClick(View v) {
        switch (v.getId()) {
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
    public void onProgressChanged(SeekBar seekBar, int progress, boolean fromUser) {}

    @Override
    public void onStartTrackingTouch(SeekBar seekBar) {}

    @Override
    public void onStopTrackingTouch(SeekBar seekBar) {
        switch (seekBar.getId()) {
            case R.id.seek_bar_contrast: {
                mContrastFilter.adjustProgress(seekBar.getProgress());
                mRender.requestRender();
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
        if (mRender == null || !mRender.isInitialized()) {
            mRender = new RenderEngine();
            mRender.onSurfaceCreate(holder.getSurface(), getRenderAdapter());
        } else {
            mRender.onResume(holder.getSurface());
        }
    }

    @Override
    public void surfaceChanged(@NonNull SurfaceHolder holder, int format, int width, int height) {
        LogUtil.i(TAG, "surfaceChanged: width = " + width + ", height = " + height);
        if (mRender != null) {
            mRender.onSurfaceChange(width, height);
            mRender.setResource(getApplicationContext(), R.drawable.lenna);
            //we should explicitly call requestRender() when surface size is changed
            mRender.requestRender();
        }
    }

    @Override
    public void surfaceDestroyed(@NonNull SurfaceHolder holder) {
        LogUtil.i(TAG, "surfaceDestroyed");
        //don't release Render here, we should release render when we do not need it, such as in onDestroy()
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
            };
        }
        return mRenderAdapter;
    }

    private void hideAllTab() {
        mAdjustBeautyRoot.setVisibility(View.GONE);
    }

    private void handleClickBeauty() {
        if (anyTabShow()) {
            hideAllTab();
            return;
        }
        boolean show = mAdjustBeautyRoot.getVisibility() == View.VISIBLE;
        showTab(mAdjustBeautyRoot, !show);
        if (!show) {
            if (mContrastFilter == null || !mContrastFilter.filterValid()) {
                mContrastFilter = new ContrastFilter();
                mRender.addBeautyFilter(mContrastFilter);
                mRender.requestRender();
            }
        }
    }

    private void initView() {
        mSurfaceView = findViewById(R.id.render_surface);
        mSurfaceView.setZOrderOnTop(false);
        mSurfaceView.getHolder().setFormat(PixelFormat.TRANSPARENT);
        mSurfaceView.getHolder().addCallback(this);

        mAdjustBeautyRoot = findViewById(R.id.layout_adjust_beauty_root);
        mContrastSeekBar = findViewById(R.id.seek_bar_contrast);

        mContrastSeekBar.setOnSeekBarChangeListener(this);
        findViewById(R.id.tab_beauty).setOnClickListener(this);
    }

    private void initData() {
        EngineEnv.init(getApplicationContext().getAssets());
        initDefaultVal();
    }

    private void initDefaultVal() {
        if (mContrastSeekBar != null) { mContrastSeekBar.setProgress(50); }
    }

    private void showTab(View view, boolean show) {
        view.setVisibility(show ? View.VISIBLE : View.GONE);
    }
}