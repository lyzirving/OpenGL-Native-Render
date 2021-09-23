package com.render.engine.camera;

import android.annotation.SuppressLint;
import android.content.Context;
import android.graphics.ImageFormat;
import android.graphics.Point;
import android.graphics.SurfaceTexture;
import android.hardware.camera2.CameraAccessException;
import android.hardware.camera2.CameraCaptureSession;
import android.hardware.camera2.CameraCharacteristics;
import android.hardware.camera2.CameraDevice;
import android.hardware.camera2.CameraManager;
import android.hardware.camera2.CameraMetadata;
import android.hardware.camera2.CaptureRequest;
import android.hardware.camera2.params.StreamConfigurationMap;
import android.os.Handler;
import android.os.HandlerThread;
import android.text.TextUtils;
import android.util.Size;
import android.view.Display;
import android.view.Surface;

import androidx.annotation.NonNull;

import com.render.engine.util.LogUtil;

import java.util.ArrayList;
import java.util.Arrays;
import java.util.Collections;
import java.util.Comparator;
import java.util.List;

public class CameraHelper {
    private static final String TAG = "CameraHelper";

    private volatile static CameraHelper sInstance;
    private String mCameraId;
    private int mFrontType;
    private Size mPreviewSize, mLargestOutputSize;

    private SurfaceTexture mOesTexture;
    private CaptureRequest.Builder mPreviewRequestBuilder;
    private CameraCaptureSession mCaptureSession;
    private CameraDevice mCamera;
    private HandlerThread mCameraThread;
    private Handler mCameraHandler;

    private CameraDevice.StateCallback mCameraStateCallback = new CameraDevice.StateCallback() {
        @Override
        public void onOpened(@NonNull CameraDevice camera) {
            LogUtil.i(TAG, "onOpened");
            mCamera = camera;
            createCameraPreviewSession();
        }

        @Override
        public void onDisconnected(@NonNull CameraDevice camera) {
            LogUtil.e(TAG, "onDisconnected");
            closeCamera();
        }

        @Override
        public void onError(@NonNull CameraDevice camera, int error) {
            LogUtil.e(TAG, "onError: code = " + error);
            closeCamera();
        }

        @Override
        public void onClosed(@NonNull CameraDevice camera) {
            super.onClosed(camera);
            LogUtil.i(TAG, "onClosed");
        }
    };

    private CameraCaptureSession.StateCallback mCaptureSessionCallback = new CameraCaptureSession.StateCallback() {
        @Override
        public void onConfigured(@NonNull CameraCaptureSession session) {
            try {
                LogUtil.i(TAG, "onConfigured");
                mCaptureSession = session;
                // Auto focus should be continuous for camera preview.
                mPreviewRequestBuilder.set(CaptureRequest.CONTROL_AF_MODE, CaptureRequest.CONTROL_AF_MODE_CONTINUOUS_PICTURE);
                // Flash is automatically enabled when necessary.
                /*if (mFlashSupport) {
                    mPreviewRequestBuilder.set(CaptureRequest.CONTROL_AE_MODE,
                            CaptureRequest.CONTROL_AE_MODE_ON_AUTO_FLASH);
                }*/
                // Finally, we start displaying the camera preview.
                CaptureRequest previewRequest = mPreviewRequestBuilder.build();
                //the preview data will continuously send data to SurfaceTexture
                //the onFrameAvailable callback will be called
                mCaptureSession.setRepeatingRequest(previewRequest, mCaptureCallback, mCameraHandler);
            } catch (Exception e) {
                LogUtil.i(TAG, "onConfigured: exception = " + e.getMessage());
                e.printStackTrace();
                closeCamera();
            }
        }

        @Override
        public void onConfigureFailed(@NonNull CameraCaptureSession session) {
            LogUtil.e(TAG, "onConfigureFailed");
            closeCamera();
        }
    };

    private CameraCaptureSession.CaptureCallback mCaptureCallback = new CameraCaptureSession.CaptureCallback() {};

    private CameraHelper() {}

    public static CameraHelper get() {
        if (sInstance == null) {
            synchronized (CameraHelper.class) {
                if (sInstance == null) {
                    sInstance = new CameraHelper();
                }
            }
        }
        return sInstance;
    }

    public void closeCamera() {
        try {
            LogUtil.i(TAG, "closeCamera");
            if (null != mCaptureSession) { mCaptureSession.close(); }
            if (null != mCamera) { mCamera.close(); }
            releaseOesTexture();
        } catch (Exception e) {
            LogUtil.e(TAG, "closeCamera: exception happens, " + e.getMessage());
            e.printStackTrace();
        } finally {
            mCamera = null;
            mCaptureSession = null;
        }
    }

    public Size getPreviewSize() { return mPreviewSize; }

    public int getFrontType() { return mFrontType; }

    @SuppressLint("MissingPermission")
    public void open(Context ctx) {
        if (TextUtils.isEmpty(mCameraId) || mCameraHandler == null) {
            throw new RuntimeException("camera is not prepared");
        }
        try {
            CameraManager manager = (CameraManager) ctx.getSystemService(Context.CAMERA_SERVICE);
            manager.openCamera(mCameraId, mCameraStateCallback, mCameraHandler);
        } catch (Exception e) {
            LogUtil.e(TAG, "openCamera: failed to open, msg = " + e.getMessage());
            e.printStackTrace();
        }
    }


    public void prepare(Context ctx, int surfaceWidth, int surfaceHeight) {
        prepare(ctx, surfaceWidth, surfaceHeight, CameraMetadata.LENS_FACING_BACK);
    }

    public void prepare(Context ctx, int surfaceWidth, int surfaceHeight, int cameraFaceInt) {
        mFrontType = cameraFaceInt;
        CameraManager manager = (CameraManager) ctx.getSystemService(Context.CAMERA_SERVICE);
        Display display = ctx.getDisplay();

        CameraCharacteristics character = getCameraAttr(manager, cameraFaceInt);
        if (character == null) { throw new RuntimeException("no support camera face " + cameraFaceInt); }

        StreamConfigurationMap map = character.get(CameraCharacteristics.SCALER_STREAM_CONFIGURATION_MAP);
        if (map == null) { throw new RuntimeException("no configuration map for camera face " + cameraFaceInt); }

        Size largestOutputSizeJpeg = Collections.max(Arrays.asList(map.getOutputSizes(ImageFormat.JPEG)), new CompareSizesByArea());

        //phone's natural rotation is vertical-stand, i-pad's natural direction is horizontal-stand
        //display rotation is the rotation from device's natural stand
        int displayRotation = display.getRotation();
        Integer sensorOrientation = character.get(CameraCharacteristics.SENSOR_ORIENTATION);
        if (sensorOrientation == null) { sensorOrientation = cameraFaceInt == CameraMetadata.LENS_FACING_FRONT ? 270 : 0; }
        boolean swapDimension = needSwapDimension(displayRotation, sensorOrientation);
        LogUtil.i(TAG, "prepare: display rotation = " + displayRotation + ", sensor rotation = " + sensorOrientation + ", need swap dimension = " + swapDimension);

        Size previewSize = getPreviewSize(display, map, swapDimension, surfaceWidth, surfaceHeight, largestOutputSizeJpeg);
        if (swapDimension) {
            mPreviewSize = new Size(previewSize.getHeight(), previewSize.getWidth());
            mLargestOutputSize = new Size(largestOutputSizeJpeg.getHeight(), largestOutputSizeJpeg.getWidth());
        } else {
            mPreviewSize = previewSize;
            mLargestOutputSize = largestOutputSizeJpeg;
        }
        startCameraThread();
        LogUtil.i(TAG, "prepare: largest jpeg output size = (" + mLargestOutputSize.getWidth() + ", " + mLargestOutputSize.getHeight() + ")"
                + ", preview size = (" + mPreviewSize.getWidth() + ", " + mPreviewSize.getHeight() + ")");
    }

    public void release() {
        LogUtil.i(TAG, "release");
        closeCamera();
        stopCameraThread();
        mCaptureSession = null;
        mCamera = null;
        mCameraId = null;
    }

    public void setOesTexture(SurfaceTexture surfaceTexture) {
        mOesTexture = surfaceTexture;
    }

    public void setOnFrameAvailableListener(SurfaceTexture.OnFrameAvailableListener listener) {
        if (mOesTexture == null) { throw new RuntimeException("SurfaceTexture is not prepared"); }
        if (mCameraHandler == null) { throw new RuntimeException("camera env is not prepared"); }
        mOesTexture.setOnFrameAvailableListener(listener, mCameraHandler);
    }

    private void createCameraPreviewSession() {
        try {
            mOesTexture.setDefaultBufferSize(mPreviewSize.getWidth(), mPreviewSize.getHeight());
            // This is the output Surface we need to start preview.
            Surface surface = new Surface(mOesTexture);
            mPreviewRequestBuilder = mCamera.createCaptureRequest(CameraDevice.TEMPLATE_PREVIEW);
            mPreviewRequestBuilder.addTarget(surface);
            mCamera.createCaptureSession(Collections.singletonList(surface), mCaptureSessionCallback, mCameraHandler);
        } catch (Exception e) {
            LogUtil.d(TAG, "createCameraPreviewSession: exception happens, " + e.getMessage());
            e.printStackTrace();
            closeCamera();
        }
    }

    /**
     * Given {@code choices} of {@code Size}s supported by a camera, choose the smallest one that
     * is at least as large as the respective texture view size, and that is at most as large as the
     * respective max size, and whose aspect ratio matches with the specified value. If such size
     * doesn't exist, choose the largest one that is at most as large as the respective max size,
     * and whose aspect ratio matches with the specified value.
     *
     * @param choices           The list of sizes that the camera supports for the intended output
     *                          class
     * @param textureViewWidth  The width of the texture view relative to sensor coordinate
     * @param textureViewHeight The height of the texture view relative to sensor coordinate
     * @param maxWidth          The maximum width that can be chosen
     * @param maxHeight         The maximum height that can be chosen
     * @param aspectRatio       The aspect ratio
     * @return The optimal {@code Size}, or an arbitrary one if none were big enough
     */
    private static Size chooseOptimalSize(Size[] choices, int textureViewWidth,
                                          int textureViewHeight, int maxWidth, int maxHeight, Size aspectRatio) {

        if (choices != null) {
            StringBuilder sb = new StringBuilder();
            for (int i = 0; i < choices.length; i++) {
                sb.append("index: ").append(i).append("(").append(choices[i].getWidth()).append(",")
                        .append(choices[i].getHeight()).append(")").append("\n");
            }
            LogUtil.i(TAG, "chooseOptimalSize: \n" + sb.toString());
        }
        // Collect the supported resolutions that are at least as big as the preview Surface
        List<Size> bigEnough = new ArrayList<>();
        // Collect the supported resolutions that are smaller than the preview Surface
        List<Size> notBigEnough = new ArrayList<>();
        int w = aspectRatio.getWidth();
        int h = aspectRatio.getHeight();
        for (Size option : choices) {
            if (option.getWidth() <= maxWidth && option.getHeight() <= maxHeight &&
                    option.getHeight() == option.getWidth() * h / w) {
                if (option.getWidth() >= textureViewWidth &&
                        option.getHeight() >= textureViewHeight) {
                    bigEnough.add(option);
                } else {
                    notBigEnough.add(option);
                }
            }
        }

        // Pick the smallest of those big enough. If there is no one big enough, pick the
        // largest of those not big enough.
        if (bigEnough.size() > 0) {
            return Collections.min(bigEnough, new CompareSizesByArea());
        } else if (notBigEnough.size() > 0) {
            return Collections.max(notBigEnough, new CompareSizesByArea());
        } else {
            LogUtil.e(TAG, "chooseOptimalSize: couldn't find any suitable preview size");
            return choices[0];
        }
    }

    private CameraCharacteristics getCameraAttr(CameraManager manager, int cameraFaceInt) {
        try {
            CameraCharacteristics result;
            Integer lensFace;
            for (String cameraId : manager.getCameraIdList()) {
                result = manager.getCameraCharacteristics(cameraId);
                lensFace = result.get(CameraCharacteristics.LENS_FACING);
                if (lensFace != null && lensFace == cameraFaceInt) {
                    LogUtil.i(TAG, "getCameraAttr: camera id = " + cameraId);
                    mCameraId = cameraId;
                    return result;
                }
            }
        } catch (Exception e) {
            LogUtil.e(TAG, "getCameraAttr: exception, " + e.getMessage());
            e.printStackTrace();
        }
        return null;
    }

    private Size getPreviewSize(Display display, StreamConfigurationMap map, boolean swapDimension,
                                int viewWidth, int viewHeight, Size largestJpegOutputSize) {
        Point windowSize = new Point();
        display.getSize(windowSize);
        int maxPreviewWidth = windowSize.x;
        int maxPreviewHeight = windowSize.y;
        int rotatedPreviewWidth = viewWidth;
        int rotatedPreviewHeight = viewHeight;
        if (swapDimension) {
            rotatedPreviewWidth = viewHeight;
            rotatedPreviewHeight = viewWidth;
            maxPreviewWidth = windowSize.y;
            maxPreviewHeight = windowSize.x;
        }
        LogUtil.i(TAG, "getPreviewSize: need swap dimension = " + swapDimension
                + ", rotated preview size = (" + rotatedPreviewWidth + "," + rotatedPreviewHeight + ")"
                + ", rotated max view size = (" + maxPreviewWidth + "," + maxPreviewHeight + ")"
                + ", largest out put size = (" + largestJpegOutputSize.getWidth() + "," + largestJpegOutputSize.getHeight() + ")");
        return chooseOptimalSize(map.getOutputSizes(SurfaceTexture.class),
                rotatedPreviewWidth, rotatedPreviewHeight, maxPreviewWidth,
                maxPreviewHeight, largestJpegOutputSize);
    }

    private boolean needSwapDimension(int displayRotation, int sensorOrientation) {
        switch (displayRotation) {
            case Surface.ROTATION_0:
            case Surface.ROTATION_180: {
                return sensorOrientation == 90 || sensorOrientation == 270;
            }
            case Surface.ROTATION_90:
            case Surface.ROTATION_270: {
                return sensorOrientation == 0 || sensorOrientation == 180;
            }
            default: {
                LogUtil.i(TAG, "needSwapDimension: default");
                return false;
            }
        }
    }

    private synchronized void releaseOesTexture() {
        if (mOesTexture != null) {
            if (!mOesTexture.isReleased()) { mOesTexture.release(); }
            mOesTexture.setOnFrameAvailableListener(null);
        }
        mOesTexture = null;
    }

    private void startCameraThread() {
        mCameraThread = new HandlerThread(TAG);
        mCameraThread.start();
        mCameraHandler = new Handler(mCameraThread.getLooper());
    }

    private void stopCameraThread() {
        try {
            mCameraThread.quitSafely();
            mCameraThread.join();
            mCameraThread = null;
            mCameraHandler = null;
        } catch (Exception e) {
            LogUtil.e(TAG, "stopCameraThread: exception, " + e.getMessage());
            e.printStackTrace();
        }
    }

    /**
     * Compares two {@code Size}s based on their areas.
     */
    private static class CompareSizesByArea implements Comparator<Size> {
        @Override
        public int compare(Size lhs, Size rhs) {
            // We cast here to ensure the multiplications won't overflow
            return Long.signum((long) lhs.getWidth() * lhs.getHeight() -
                    (long) rhs.getWidth() * rhs.getHeight());
        }
    }
}
