//
// Created by liuyuzhou on 2021/9/8.
//
#ifndef RENDERDEMO_BACKGROUNDFILTER_H
#define RENDERDEMO_BACKGROUNDFILTER_H

#include <jni.h>
#include "BaseFilter.h"

class BackgroundFilter : public BaseFilter {
public:
    void destroy() override;
    int getBitmapWidth();
    int getBitmapHeight();
    void init() override;
    void initBuffer() override;
    void initFrameBuffer() override;
    void initTexture() override;
    GLint onDraw(GLint inputTextureId) override;
    void setBitmap(JNIEnv* env, jobject bitmap);
private:
    unsigned char* mPixel = nullptr;
    int mBitmapWidth{0};
    int mBitmapHeight{0};

    GLuint mFrameBufferId{0};
    GLuint mFrameBufferTextureId{0};
};

#endif //RENDERDEMO_BACKGROUNDFILTER_H
