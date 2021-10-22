//
// Created by liuyuzhou on 2021/9/2.
//
#include "GlUtil.h"
#include "LogUtil.h"
#include "JniUtil.h"
#include "ModelLoader.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include <pthread.h>

#define JAVA_CLASS "com/render/engine/core/EngineEnv"
#define TAG "GlUtil"
#define NO_TEXTURE 0

static GlUtil *gGlUtil = nullptr;
static JavaVM *gJvm{nullptr};
static pthread_mutex_t gMutex;

GlUtil::GlUtil() {
    LogUtil::logI(TAG, {"construct"});
}

GlUtil::~GlUtil() {
    LogUtil::logI(TAG, {"deconstruct"});
}

void *envInitTask(void *args) {
    JNIEnv *env = nullptr;
    if (!JniUtil::self()->attachJvm(&env)) {
        LogUtil::logI(TAG, {"envInitTask: failed to attach thread to jvm"});
        return nullptr;
    }
    ModelLoader loader;
    std::string modelName("tiger_nose");
    bool modelExist = loader.buildLocalSource(ResourceType::OBJ, modelName.c_str());
    bool mtlExist = loader.buildLocalSource(ResourceType::MTL, modelName.c_str());
    LogUtil::logI(TAG, {"envInitTask: load model, name = ", modelName, ", model = ", std::to_string(modelExist), ", mtl = ", std::to_string(mtlExist)});
    JniUtil::self()->detachThread();
    return nullptr;
}

static void nInitUtilEnv(JNIEnv *env, jclass clazz, jobject assetManager) {
    auto pGlUtil = GlUtil::self();
    pGlUtil->setAssetsManager(env, assetManager);
    pthread_t thread;
    pthread_create(&thread, nullptr, envInitTask, nullptr);
}

static JNINativeMethod sJniMethods[] = {
        {
                "nInitEnv", "(Landroid/content/res/AssetManager;)V",
                (void *) nInitUtilEnv
        }
};

void GlUtil::destroy() {
    pthread_mutex_destroy(&gMutex);
}

GLuint GlUtil::generateTextureFromAssets(const char *path) {
    if (mAssetManager == nullptr) {
        LogUtil::logI(TAG, {"generateTextureFromAssets: path = ", path, {", assets manager is null"}});
        return NO_TEXTURE;
    }
    if (path == nullptr || strlen(path) == 0) {
        LogUtil::logI(TAG, {"generateTextureFromAssets: invalid input path"});
        return NO_TEXTURE;
    }
    AAsset *asset = AAssetManager_open(mAssetManager, path, AASSET_MODE_UNKNOWN);
    if (asset == nullptr) {
        LogUtil::logI(TAG, {"generateTextureFromAssets: failed to open asset, path = ", path});
        return NO_TEXTURE;
    }
    off_t len = AAsset_getLength(asset);
    auto*fileData = (unsigned char*) AAsset_getBuffer(asset);
    int width;
    int height;
    int channel;
    unsigned char* content = stbi_load_from_memory(fileData, len, &width, &height, &channel, 0);
    if (content == nullptr) {
        LogUtil::logI(TAG, {"generateTextureFromAssets: failed to load image from assets, path = ", path});
        return NO_TEXTURE;
    }
    GLuint textureId;
    glGenTextures(1, &textureId);
    glBindTexture(GL_TEXTURE_2D, textureId);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, content);

    LogUtil::logI(TAG, {"generateTextureFromAssets: path = ", path, ", width = ", std::to_string(width),
                        ", height = ", std::to_string(height), ", channel = ", std::to_string(channel),
                        ", texture = ", std::to_string(textureId)});
    return textureId;
}

void GlUtil::init(JNIEnv* env) {
    env->GetJavaVM(&gJvm);
    pthread_mutex_init(&gMutex, nullptr);
}

bool GlUtil::registerSelf(JNIEnv *env) {
    int count = sizeof(sJniMethods) / sizeof(sJniMethods[0]);
    jclass javaClass = env->FindClass(JAVA_CLASS);
    if (!javaClass) {
        LogUtil::logE(TAG, {"registerSelf: failed to find class ", JAVA_CLASS});
        return false;
    }
    if (env->RegisterNatives(javaClass, sJniMethods, count) < 0) {
        LogUtil::logE(TAG, {"registerSelf: failed to register native methods ", JAVA_CLASS});
        return false;
    }
    return true;
}

GLuint GlUtil::loadShader(GLenum shaderType, const char *source) {
    GLuint shader = glCreateShader(shaderType);
    if (shader) {
        glShaderSource(shader, 1, &source, nullptr);
        glCompileShader(shader);
        GLint compiled = 0;
        //check shader compiled-status
        glGetShaderiv(shader, GL_COMPILE_STATUS, &compiled);
        if (!compiled) {
            LogUtil::logI(TAG, {"loadShader: failed to compile shader, type = ", std::to_string(shaderType)});
            shader = 0;
            GLint infoLen = 0;
            glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &infoLen);
            if (infoLen) {
                char *buf = (char *) malloc(infoLen);
                glGetShaderInfoLog(shader, infoLen, nullptr, buf);
                LogUtil::logI(TAG, {"loadShader: failed to compile shader, type = ", std::to_string(shaderType), ", info = ", buf});
                glDeleteShader(shader);
                free(buf);
            }
        }
    } else {
        LogUtil::logI(TAG, {"loadShader: failed to create shader, type = ", std::to_string(shaderType), ", source = ", source});
    }
    return shader;
}

GLuint GlUtil::loadProgram(const char *vertexShader, const char *fragmentShader) {
    if (vertexShader == nullptr || fragmentShader == nullptr) {
        LogUtil::logI(TAG, {"loadProgram: input is invalid"});
        return 0;
    }
    GLuint vertexShaderId = loadShader(GL_VERTEX_SHADER, vertexShader);
    if (vertexShaderId == 0) {
        LogUtil::logI(TAG, {"loadProgram: failed to load vertex shader"});
        return 0;
    }
    GLuint fragmentShaderId = loadShader(GL_FRAGMENT_SHADER, fragmentShader);
    if (fragmentShaderId == 0) {
        LogUtil::logI(TAG, {"loadProgram: failed to load fragment shader"});
        glDeleteShader(vertexShaderId);
        return 0;
    }
    GLuint program = glCreateProgram();
    if (program) {
        glAttachShader(program, vertexShaderId);
        glAttachShader(program, fragmentShaderId);
        glLinkProgram(program);
        GLint linkStatus = GL_FALSE;
        glGetProgramiv(program, GL_LINK_STATUS, &linkStatus);
        if (linkStatus != GL_TRUE) {
            LogUtil::logI(TAG, {"loadProgram: failed to link program"});
            program = 0;
            GLint bufLength = 0;
            glGetProgramiv(program, GL_INFO_LOG_LENGTH, &bufLength);
            if (bufLength) {
                char* buf = (char*) malloc(bufLength);
                glGetProgramInfoLog(program, bufLength, nullptr, buf);
                LogUtil::logI(TAG, {"loadProgram: failed to link program, into = ", buf});
                free(buf);
            }
            glDeleteProgram(program);
        }
    } else {
        LogUtil::logI(TAG, {"loadProgram: failed to create program"});
    }
    return program;
}

void GlUtil::release() {}

char* GlUtil::readAssets(const char *path) {
    if (mAssetManager == nullptr) {
        LogUtil::logI(TAG, {"readAssets: path = ", path, {", assets manager is null"}});
        return nullptr;
    }
    if (path == nullptr || strlen(path) == 0) {
        LogUtil::logI(TAG, {"readAssets: invalid input path"});
        return nullptr;
    }
    AAsset *asset = AAssetManager_open(mAssetManager, path, AASSET_MODE_BUFFER);
    if (asset == nullptr) {
        LogUtil::logI(TAG, {"readAssets: failed to open asset, path = ", path});
        return nullptr;
    }
    off_t len = AAsset_getLength(asset);
    char *result = static_cast<char *>(malloc(len + 1));
    result[len] = 0;
    AAsset_read(asset, result, len);
    AAsset_close(asset);

    return result;
}

char * GlUtil::readAssets(const char *path, int* size) {
    if (mAssetManager == nullptr) {
        LogUtil::logI(TAG, {"readAssets: path = ", path, {", assets manager is null"}});
        *size = 0;
        return nullptr;
    }
    if (path == nullptr || strlen(path) == 0) {
        LogUtil::logI(TAG, {"readAssets: invalid input path"});
        *size = 0;
        return nullptr;
    }
    AAsset *asset = AAssetManager_open(mAssetManager, path, AASSET_MODE_BUFFER);
    if (asset == nullptr) {
        LogUtil::logI(TAG, {"readAssets: failed to open asset, path = ", path});
        *size = 0;
        return nullptr;
    }
    *size = AAsset_getLength(asset);
    char *result = static_cast<char *>(malloc(*size + 1));
    result[*size] = 0;
    AAsset_read(asset, result, *size);
    AAsset_close(asset);

    return result;
}

GlUtil *GlUtil::self() {
    if (gGlUtil == nullptr) {
        pthread_mutex_lock(&gMutex);
        if (gGlUtil == nullptr) { gGlUtil = new GlUtil; }
        pthread_mutex_unlock(&gMutex);
    }
    return gGlUtil;
}

void GlUtil::setAssetsManager(JNIEnv *env, jobject assetManager) {
    if (mAssetManager == nullptr) {
        LogUtil::logI(TAG, {"setAssetsManager: set assets manager from java"});
        mAssetManager = AAssetManager_fromJava(env, assetManager);
        if (mAssetManager == nullptr) {
            LogUtil::logI(TAG, {"setAssetsManager: get null asset manager from java"});
        }
    }
}

