//
// Created by liuyuzhou on 2021/9/7.
//
#ifndef ENGINE_RENDER_H
#define ENGINE_RENDER_H

#include "GlUtil.h"
#include "RenderEglBase.h"
#include "EventQueue.h"
#include "WorkQueue.h"

#include "BaseFilterGroup.h"
#include "BackgroundFilter.h"
#include "ScreenFilter.h"
#include "MaskFilter.h"

class Render {
public:
    static bool registerSelf(JNIEnv *env);

    Render();
    ~Render();

    bool addBeautyFilter(const char* filterType, bool buildInitTask);
    void adjust(const char* filterType, int progress);
    void adjustProp(const char* filterType, const char* prop, int progress);
    void clearBeautyFilter();
    void drawFrame();
    void deleteFilters();
    void enqueueMessage(EventType what);
    bool initialized();
    void notifyEnvPrepare(JNIEnv* env, jobject listener);
    void notifyEnvRelease(JNIEnv* env, jobject listener);
    void render(JNIEnv* env);
    void release(JNIEnv* env);
    void runBeforeDraw();
    void setResource(JNIEnv* env, jobject bitmap);
    void setSize(GLint width, GLint height);
    void setNativeWindow(ANativeWindow* window);

private:
    RenderEglBase* mEglCore;
    EventQueue* mEvtQueue;
    WorkQueue* mWorkQueue;
    GLint mWidth{0};
    GLint mHeight{0};
    RenderStatus mStatus = RenderStatus::STATUS_IDLE;

    BaseFilterGroup* mBeautyFilterGroup = nullptr;
    BackgroundFilter* mBackgroundFilter = nullptr;
    ScreenFilter* mScreenFilter = nullptr;
    MaskFilter* mMaskFilter = nullptr;
};

#endif //ENGINE_RENDER_H
