//
// Created by liuyuzhou on 2021/9/2.
//
#include "LogUtil.h"
#include "Static.h"

#define JAVA_CLASS "com/render/engine/core/EngineEnv"
#define TAG "GlUtil"

static void nInitUtilEnv(JNIEnv *env, jclass clazz, jobject assetManager) {
    auto pGlUtil = GlUtil::self();
    pGlUtil->setAssetsManager(env, assetManager);
}

static JNINativeMethod sJniMethods[] = {
        {
                "nInitEnv", "(Landroid/content/res/AssetManager;)V",
                (void *) nInitUtilEnv
        }
};

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

GlUtil::GlUtil() {
    LogUtil::logI(TAG, {"construct"});
}

GlUtil::~GlUtil() {
    LogUtil::logI(TAG, {"deconstruct"});
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
        LogUtil::logI(TAG, {"loadShader: failed to create shader, type = ", std::to_string(shaderType)});
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

bool GlUtil::mapContains(std::map<jlong, jobject> *pMap, jlong key) {
    auto iterator = pMap->find(key);
    if (iterator == pMap->end()) {
        return false;
    } else {
        return true;
    }
}

void GlUtil::release() {}

char *GlUtil::readAssets(const char *path) {
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

GlUtil *GlUtil::self() {
    if (EngineUtil::gGlUtil == nullptr) {
        EngineUtil::lock();
        if (EngineUtil::gGlUtil == nullptr) {
            EngineUtil::gGlUtil = new GlUtil;
        }
        EngineUtil::unlock();
    }
    return EngineUtil::gGlUtil;
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

