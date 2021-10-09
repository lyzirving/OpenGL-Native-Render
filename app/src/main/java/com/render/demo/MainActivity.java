package com.render.demo;

import androidx.annotation.NonNull;
import androidx.annotation.Nullable;
import androidx.core.app.ActivityCompat;

import android.Manifest;
import android.content.Intent;
import android.content.pm.PackageManager;
import android.os.Bundle;
import android.view.View;
import android.widget.Toast;

import com.render.demo.ui.BaseActivity;
import com.render.demo.ui.CameraActivity;
import com.render.demo.ui.StillImageActivity;
import com.render.engine.core.EngineEnv;
import com.render.engine.util.LogUtil;

/**
 * @author lyzirving
 */
public class MainActivity extends BaseActivity implements View.OnClickListener {
    private static final String TAG = "MainActivity";
    private static final int REQUEST_WRITE_PERMISSION = 0x01;
    private static final int REQUEST_CAMERA_PERMISSION = 0x02;

    @Override
    public void onClick(View v) {
        switch (v.getId()) {
            case R.id.btn_start_still_image: {
                gotoPage(StillImageActivity.class);
                break;
            }
            case R.id.btn_camera: {
                gotoPage(CameraActivity.class);
                break;
            }
            default: {
                break;
            }
        }
    }

    @Override
    public void onRequestPermissionsResult(int requestCode, @NonNull String[] permissions, @NonNull int[] grantResults) {
        super.onRequestPermissionsResult(requestCode, permissions, grantResults);
        switch (requestCode) {
            case REQUEST_WRITE_PERMISSION: {
                if (grantResults[0] == PackageManager.PERMISSION_GRANTED && grantResults[1] == PackageManager.PERMISSION_GRANTED) {
                    LogUtil.i(TAG, "onRequestPermissionsResult: read-write permission granted");
                    EngineEnv.init(getApplicationContext());
                    int cameraPermission = ActivityCompat.checkSelfPermission(getApplicationContext(), Manifest.permission.CAMERA);
                    if (cameraPermission != PackageManager.PERMISSION_GRANTED) {
                        ActivityCompat.requestPermissions(this, new String[]{Manifest.permission.CAMERA},
                                REQUEST_CAMERA_PERMISSION);
                    }
                } else {
                    Toast.makeText(getApplicationContext(), "读写权限没有授予", Toast.LENGTH_SHORT).show();
                }
                break;
            }
            case REQUEST_CAMERA_PERMISSION: {
                if (grantResults[0] == PackageManager.PERMISSION_GRANTED) {
                    LogUtil.i(TAG, "onRequestPermissionsResult: camera permission granted");
                } else {
                    Toast.makeText(getApplicationContext(), "相机权限没有授予", Toast.LENGTH_SHORT).show();
                }
                break;
            }
            default: {
                break;
            }
        }
    }

    @Override
    protected int getLayoutId() {
        return R.layout.activity_main;
    }

    @Override
    protected void initView() {
        findViewById(R.id.btn_start_still_image).setOnClickListener(this);
        findViewById(R.id.btn_camera).setOnClickListener(this);
    }

    @Override
    protected void initData() {}

    @Override
    protected void release() {
        LogUtil.i(TAG, "release");
        EngineEnv.release();
    }

    @Override
    protected void onPostCreate(@Nullable Bundle savedInstanceState) {
        super.onPostCreate(savedInstanceState);
        LogUtil.i(TAG, "onPostCreate");
        requestUserPermission();
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
    }

    @Override
    protected void onStop() {
        super.onStop();
        LogUtil.i(TAG, "onStop");
    }

    private void gotoPage(Class where) {
        Intent intent = new Intent();
        intent.setClass(getApplicationContext(), where);
        startActivity(intent);
    }

    private void requestUserPermission() {
        int writePermission = ActivityCompat.checkSelfPermission(getApplicationContext(), Manifest.permission.WRITE_EXTERNAL_STORAGE);
        if (writePermission != PackageManager.PERMISSION_GRANTED) {
            ActivityCompat.requestPermissions(this,
                    new String[]{Manifest.permission.WRITE_EXTERNAL_STORAGE, Manifest.permission.READ_EXTERNAL_STORAGE},
                    REQUEST_WRITE_PERMISSION);
        } else {
            LogUtil.i(TAG, "requestUserPermission");
            EngineEnv.init(getApplicationContext());
            int cameraPermission = ActivityCompat.checkSelfPermission(getApplicationContext(), Manifest.permission.CAMERA);
            if (cameraPermission != PackageManager.PERMISSION_GRANTED) {
                ActivityCompat.requestPermissions(this, new String[]{Manifest.permission.CAMERA},
                        REQUEST_CAMERA_PERMISSION);
            }
        }
    }

}