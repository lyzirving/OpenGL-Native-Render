//
// Created by liuyuzhou on 2021/9/7.
//
#ifndef ENGINE_GLUTIL_H
#define ENGINE_GLUTIL_H

#include <jni.h>
#include <GLES2/gl2.h>
#include <android/asset_manager.h>
#include <android/asset_manager_jni.h>

class GlUtil {
public:
    static void destroy();
    static void init();
    static bool registerSelf(JNIEnv *env);
    static GlUtil* self();

    GLuint loadProgram(const char* vertexShader, const char* fragmentShader);
    GLuint loadShader(GLenum shaderType, const char* source);
    char* readAssets(const char* path);
    void release();
    void setAssetsManager(JNIEnv* env, jobject assetManager);

private:
    GlUtil();
    ~GlUtil();

    AAssetManager* mAssetManager = nullptr;
};

#endif //ENGINE_GLUTIL_H
