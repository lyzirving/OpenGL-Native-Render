//
// Created by liuyuzhou on 2021/10/22.
//
#include "ModelFilter.h"
#include "GlUtil.h"
#include "MatrixUtil.h"
#include "LogUtil.h"

#define TAG "ModelFilter"

void ModelFilter::initBuffer() {
    if (mObjGroup == nullptr) {
        LogUtil::logI(TAG, {"initBuffer: obj group is null"});
    } else {
        delete mMvpMatrix;
        delete mModelMatrix;
        delete mProjectionMatrix;
        mMvpMatrix = new GLfloat[16];
        mProjectionMatrix = new GLfloat[16];
        mModelMatrix = new GLfloat[16];
        MatrixUtil::setIdentityM(mMvpMatrix, 0);
        MatrixUtil::setIdentityM(mProjectionMatrix, 0);
        MatrixUtil::setIdentityM(mModelMatrix, 0);

        int objSize = mObjGroup->getObjSize();
        LogUtil::logI(TAG, {"initBuffer: obj group size = ", std::to_string(objSize)});

        mVertexBufferId = new GLuint[objSize];
        mTextureCoordBufferId = new GLuint[objSize];
        mVertexNormalBufferId = new GLuint[objSize];

        glGenBuffers(objSize, mVertexBufferId);
        glGenBuffers(objSize, mTextureCoordBufferId);
        glGenBuffers(objSize, mVertexNormalBufferId);

        Obj3D* obj{nullptr};

        for (int i = 0; i < objSize; ++i) {
            obj = mObjGroup->getObj(i);

            LogUtil::logI(TAG, {"initBuffer: vertex 1 = ", std::to_string(obj->getVertexArray()[1])});
            glBindBuffer(GL_ARRAY_BUFFER, mVertexBufferId[i]);
            glBufferData(GL_ARRAY_BUFFER, obj->getVertexCount() * 3 * sizeof(GLfloat), obj->getVertexArray(), GL_STATIC_DRAW);

            LogUtil::logI(TAG, {"initBuffer: texture 1 = ", std::to_string(obj->getTextureArray()[1])});
            glBindBuffer(GL_ARRAY_BUFFER, mTextureCoordBufferId[i]);
            glBufferData(GL_ARRAY_BUFFER, obj->getTextureCoordCount() * 2 * sizeof(GLfloat), obj->getTextureArray(), GL_STATIC_DRAW);

            LogUtil::logI(TAG, {"initBuffer: normal 1 = ", std::to_string(obj->getVertexNormalArray()[1])});
            glBindBuffer(GL_ARRAY_BUFFER, mVertexNormalBufferId[i]);
            glBufferData(GL_ARRAY_BUFFER, obj->getVertexNormalCount() * 3 * sizeof(GLfloat), obj->getVertexNormalArray(), GL_STATIC_DRAW);
        }

        glBindBuffer(GL_ARRAY_BUFFER, 0);
    }
}

void ModelFilter::initHandler() {
    BaseFilter::initHandler();
    mVertexNormalHandler = glGetAttribLocation(mProgram, "aVertexNormal");
    mMVPMatrixHandler = glGetUniformLocation(mProgram, "uMVPMatrix");
    mMMatrixHandler = glGetUniformLocation(mProgram, "uMMatrix");
    mLightLocationHandler = glGetUniformLocation(mProgram, "uLightLocation");
    mAmbientHandler = glGetUniformLocation(mProgram, "aAmbient");
    mDiffuseHandler = glGetUniformLocation(mProgram, "uDiffuse");
}

void ModelFilter::initFrameBuffer() {
    if (mFrameBufferId != 0) { glDeleteFramebuffers(1, &mFrameBufferId); }
    if (mTextureId != 0) { glDeleteTextures(1, &mTextureId); }

    glGenFramebuffers(1, &mFrameBufferId);
    glGenTextures(1, &mTextureId);
    glBindTexture(GL_TEXTURE_2D, mTextureId);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, mWidth, mHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE,nullptr);

    glBindFramebuffer(GL_FRAMEBUFFER, mFrameBufferId);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, mTextureId, 0);

    glBindTexture(GL_TEXTURE_2D, 0);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void ModelFilter::initTexture() {
    BaseFilter::initTexture();
    int objSize = mObjGroup->getObjSize();
    if (objSize > 0) {
        LogUtil::logI(TAG, {"initTexture: obj size = ", std::to_string(objSize)});
        mModelTexture = new GLuint[objSize];
        MtlLib* lib;
        for (int i = 0; i < objSize; ++i) {
            Obj3D* obj = mObjGroup->getObj(i);
            if ((lib = obj->getMtlLib()) != nullptr) {
                std::string textureName("texture/");
                textureName += lib->getTextureName();
                mModelTexture[i] = GlUtil::self()->generateTextureFromAssets(textureName.c_str());
            }
        }
    } else {
        LogUtil::logI(TAG, {"initTexture: invalid state"});
    }
}

void ModelFilter::loadShader() {
    auto gUtil = GlUtil::self();
    if (mVertexShader == nullptr) { mVertexShader = gUtil->readAssets("shader/model_vertex_shader.glsl"); }
    if (mTextureShader == nullptr) { mTextureShader = gUtil->readAssets("shader/model_fragment_shader.glsl"); }
}

GLint ModelFilter::onDraw(GLint inputTextureId) {
    glUseProgram(mProgram);

    MatrixUtil::multiplyMM(mMvpMatrix, mProjectionMatrix, mModelMatrix);
    glUniformMatrix4fv(mMVPMatrixHandler, 1, false, mMvpMatrix);
    glUniformMatrix4fv(mMMatrixHandler, 1, false, mModelMatrix);
    glUniform3f(mLightLocationHandler, 3, 0 ,3);

    int size = mObjGroup->getObjSize();
    Obj3D* obj{nullptr};
    LogUtil::logI(TAG, {"onDraw: size = ", std::to_string(size)});
    for (int i = 0; i < size; ++i) {
        obj = mObjGroup->getObj(i);

        glBindBuffer(GL_ARRAY_BUFFER, mVertexBufferId[i]);
        glVertexAttribPointer(mVertexPosHandler, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), nullptr);
        glEnableVertexAttribArray(mVertexPosHandler);

        glBindBuffer(GL_ARRAY_BUFFER, mTextureCoordBufferId[i]);
        glVertexAttribPointer(mTextureCoordinateHandler, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(GLfloat),nullptr);
        glEnableVertexAttribArray(mTextureCoordinateHandler);

        glBindBuffer(GL_ARRAY_BUFFER, mVertexNormalBufferId[i]);
        glVertexAttribPointer(mVertexNormalHandler, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), nullptr);
        glEnableVertexAttribArray(mVertexNormalHandler);

        glUniform3f(mAmbientHandler, mAmbient[i][0], mAmbient[i][1], mAmbient[i][2]);
        glUniform3f(mDiffuseHandler, mDiffuse[i][0], mDiffuse[i][1], mDiffuse[i][2]);

        glActiveTexture(GL_TEXTURE2 + i);
        glBindTexture(GL_TEXTURE_2D, mModelTexture[i]);
        glUniform1i(mTextureSamplerHandler, 2 + i);

        LogUtil::logI(TAG, {"onDraw: vertex count = ", std::to_string(obj->getVertexCount())});
        glDrawArrays(GL_TRIANGLES, 0, obj->getVertexCount());
    }

    MatrixUtil::setIdentityM(mMvpMatrix, 0);
    MatrixUtil::setIdentityM(mModelMatrix, 0);
    glDisableVertexAttribArray(mVertexPosHandler);
    glDisableVertexAttribArray(mTextureCoordinateHandler);
    glDisableVertexAttribArray(mVertexNormalHandler);
    glBindTexture(GL_TEXTURE_2D, 0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    return mTextureId;
}

void ModelFilter::postInit() {
    BaseFilter::postInit();
    MtlLib* lib{nullptr};
    int objSize = mObjGroup->getObjSize();
    if (objSize > 0) {
        delete[] mAmbient;
        delete[] mDiffuse;
        mAmbient = new GLfloat[objSize][3];
        mDiffuse = new GLfloat[objSize][3];
        LogUtil::logI(TAG, {"postInit: obj size = ", std::to_string(objSize)});
        for (int i = 0; i < objSize; ++i) {
            Obj3D* obj = mObjGroup->getObj(i);
            if ((lib = obj->getMtlLib()) != nullptr) {
                mAmbient[i][0] = lib->getAmbient()[0];
                mAmbient[i][1] = lib->getAmbient()[1];
                mAmbient[i][2] = lib->getAmbient()[2];

                mDiffuse[i][0] = lib->getDiffuse()[0];
                mDiffuse[i][1] = lib->getDiffuse()[1];
                mDiffuse[i][2] = lib->getDiffuse()[2];
            }
        }
    } else {
        LogUtil::logI(TAG, {"postInit: invalid state"});
    }
}

void ModelFilter::setObjGroup(ObjGroup *group) {
    mObjGroup = group;
}


