//
// Created by liuyuzhou on 2021/9/7.
//
#ifndef ENGINE_GLUTIL_H
#define ENGINE_GLUTIL_H

#include <jni.h>
#include <GLES3/gl3.h>
#include <android/asset_manager.h>
#include <android/asset_manager_jni.h>

class GlUtil {
public:
    static void destroy();
    static void init(JNIEnv* env);
    static bool registerSelf(JNIEnv *env);
    static GlUtil* self();

    ~GlUtil();

    GLuint generateTextureFromAssets(const char* path);
    GLuint loadProgram(const char* vertexShader, const char* fragmentShader);
    GLuint loadShader(GLenum shaderType, const char* source);
    char* readAssets(const char* path);
    char* readAssets(const char* path, int* size);
    void release();
    void setAssetsManager(JNIEnv* env, jobject assetManager);

private:
    GlUtil();

    AAssetManager* mAssetManager = nullptr;
};

#endif //ENGINE_GLUTIL_H
