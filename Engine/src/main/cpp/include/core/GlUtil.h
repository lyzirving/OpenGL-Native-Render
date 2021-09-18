//
// Created by liuyuzhou on 2021/9/7.
//
#ifndef ENGINE_GLUTIL_H
#define ENGINE_GLUTIL_H

#include <jni.h>
#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>
#include <EGL/egl.h>
#include <android/surface_texture.h>
#include <android/native_window_jni.h>
#include <android/asset_manager.h>
#include <android/asset_manager_jni.h>
#include <map>

enum class RenderStatus : uint8_t {
    STATUS_IDLE = 0,
    STATUS_PREPARE = 1,
    STATUS_RUN = 2,
    STATUS_PAUSE = 3,
    STATUS_DESTROY = 4
};

class GlUtil {
public:
    static bool registerSelf(JNIEnv *env);
    static GlUtil* self();

    GLuint loadProgram(const char* vertexShader, const char* fragmentShader);
    GLuint loadShader(GLenum shaderType, const char* source);
    bool mapContains(std::map<jlong, jobject> *pMap, jlong key);
    void release();
    char* readAssets(const char* path);
    void setAssetsManager(JNIEnv* env, jobject assetManager);
private:
    GlUtil();
    ~GlUtil();

    AAssetManager* mAssetManager = nullptr;
};

#endif //ENGINE_GLUTIL_H
