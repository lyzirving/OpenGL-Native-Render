package com.render.engine.camera;

public class RenderCamMetadata {
    private int mPreviewWidth, mPreviewHeight;
    private int mFrontType;

    private RenderCamMetadata() {}

    public static class Builder {
        private RenderCamMetadata mInner;
        public Builder() {
            mInner = new RenderCamMetadata();
        }

        public Builder previewSize(int width, int height) {
            mInner.mPreviewWidth = width;
            mInner.mPreviewHeight = height;
            return this;
        }

        public Builder frontType(int frontType) {
            mInner.mFrontType = frontType;
            return this;
        }

        public RenderCamMetadata build() {
            return mInner;
        }
    }

    public int getPreviewWidth() {
        return mPreviewWidth;
    }

    public int getPreviewHeight() {
        return mPreviewHeight;
    }

    public int getFrontType() {
        return mFrontType;
    }
}
