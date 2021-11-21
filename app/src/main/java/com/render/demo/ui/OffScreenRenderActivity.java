package com.render.demo.ui;

import android.graphics.SurfaceTexture;
import android.view.SurfaceHolder;
import android.view.SurfaceView;
import android.view.View;

import com.render.demo.R;
import com.render.engine.camera.CameraHelper;
import com.render.engine.core.OffScreenRender;
import com.render.engine.core.RenderAdapter;
import com.render.engine.util.LogUtil;

import androidx.annotation.NonNull;

/**
 * @author lyzirving
 */
public class OffScreenRenderActivity extends BaseActivity implements SurfaceHolder.Callback, View.OnClickListener {
    private static final String TAG = "OffScreenRenderActivity";

    private OffScreenRender mRender;
    private RenderAdapter mAdapter;

    private int mPreviewWidth, mPreviewHeight;
    private int mOesTextureId;

    @Override
    protected int getLayoutId() {
        return R.layout.activity_off_screen_render;
    }

    @Override
    protected void initView() {
        SurfaceView surfaceView = findViewById(R.id.off_screen_camera_view);
        surfaceView.getHolder().addCallback(this);
        findViewById(R.id.btn_open_camera).setOnClickListener(this);
        findViewById(R.id.btn_close_camera).setOnClickListener(this);
    }

    @Override
    protected void initData() {
        mRender = new OffScreenRender();
    }

    @Override
    protected void release() {
        if (mRender != null) {
            mRender.release();
        }
    }

    @Override
    public void onClick(View v) {
        switch (v.getId()) {
            case R.id.btn_open_camera: {
                openCamera();
                break;
            }
            case R.id.btn_close_camera: {
                mRender.pause();
                CameraHelper.get().closeCamera();
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
    }

    @Override
    public void surfaceChanged(@NonNull SurfaceHolder holder, int format, int width, int height) {
        LogUtil.i(TAG, "surfaceChanged: width = " + width + ", height = " + height);
        mRender.prepare(width, height, getRenderAdapter());
        mRender.setClient(holder.getSurface());
        mPreviewWidth = width;
        mPreviewHeight = height;
    }

    @Override
    public void surfaceDestroyed(@NonNull SurfaceHolder holder) {
        LogUtil.i(TAG, "surfaceDestroyed");
        mRender.pause();
    }

    private RenderAdapter getRenderAdapter() {
        if (mAdapter == null) {
            mAdapter = new RenderAdapter() {
                @Override
                public void onRenderEnvPrepare(int oesTexture) {
                    super.onRenderEnvPrepare(oesTexture);
                    LogUtil.i(TAG, "onRenderEnvPrepare: oes texture = " + oesTexture);
                    mOesTextureId = oesTexture;
                }

                @Override
                public void onRenderEnvRelease() {
                    super.onRenderEnvRelease();
                    LogUtil.i(TAG, "onRenderEnvRelease");
                    mRender.onQuit();
                    mRender = null;
                }
            };
        }
        return mAdapter;
    }

    private void openCamera() {
        if (mOesTextureId <= 0) {
            LogUtil.i(TAG, "openCamera: invalid oes texture");
            return;
        }
        if (mPreviewWidth <= 0 || mPreviewHeight <= 0) {
            LogUtil.i(TAG, "openCamera: invalid preview");
            return;
        }
        if (CameraHelper.get().isCameraOpen()) {
            LogUtil.i(TAG, "openCamera: already open");
            return;
        }
        CameraHelper.get().prepare(getApplicationContext(), mPreviewWidth, mPreviewHeight);
        SurfaceTexture surfaceTexture = new SurfaceTexture(mOesTextureId);
        mRender.setSurfaceTexture(surfaceTexture);
        CameraHelper.get().setOesTexture(surfaceTexture);
        CameraHelper.get().setOnFrameAvailableListener(mRender);
        CameraHelper.get().open(getApplicationContext());
    }
}
