//
// Created by liuyuzhou on 2021/10/22.
//
#include "ModelFilter.h"
#include "GlUtil.h"
#include "MatrixUtil.h"
#include "LogUtil.h"

#define TAG "ModelFilter"

ModelFilter::ModelFilter() {
    mMvpMatrix = new GLfloat[16];
    mProjectionMatrix = new GLfloat[16];
    mModelMatrix = new GLfloat[16];
    mViewMatrix = new GLfloat[16];
    MatrixUtil::setIdentityM(mMvpMatrix, 0);
    MatrixUtil::setIdentityM(mProjectionMatrix, 0);
    MatrixUtil::setIdentityM(mModelMatrix, 0);
    MatrixUtil::setIdentityM(mViewMatrix, 0);
}

ModelFilter::~ModelFilter() {
    delete mMvpMatrix;
    delete mProjectionMatrix;
    delete mModelMatrix;
    delete mViewMatrix;
}

void ModelFilter::destroy() {
    BaseFilter::destroy();
    int count = 0;
    if (mObjGroup == nullptr || (count = mObjGroup->getObjSize()) == 0) {
        LogUtil::logI(TAG, {"destroy: no obj"});
    } else {
        if (mVertexBufferId != nullptr) { glDeleteBuffers(count, mVertexBufferId); }
        if (mTextureCoordBufferId != nullptr) { glDeleteBuffers(count, mTextureCoordBufferId); }
        if (mVertexNormalBufferId != nullptr) { glDeleteBuffers(count, mVertexNormalBufferId); }

        delete mVertexBufferId;
        delete mTextureCoordBufferId;
        delete mVertexNormalBufferId;

        mVertexBufferId = nullptr;
        mTextureCoordBufferId = nullptr;
        mVertexNormalBufferId = nullptr;

        if (mModelTexture != nullptr) { glDeleteTextures(count, mModelTexture); }
        delete mModelTexture;
        mModelTexture = nullptr;

        mObjGroup->clearObj();
        delete mObjGroup;
    }
}

void ModelFilter::initBuffer() {
    if (mObjGroup == nullptr) {
        LogUtil::logI(TAG, {"initBuffer: obj group is null"});
    } else {
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

            glBindBuffer(GL_ARRAY_BUFFER, mVertexBufferId[i]);
            glBufferData(GL_ARRAY_BUFFER, obj->getVertexCount() * 3 * sizeof(GLfloat), obj->getVertexArray(), GL_STATIC_DRAW);

            glBindBuffer(GL_ARRAY_BUFFER, mTextureCoordBufferId[i]);
            glBufferData(GL_ARRAY_BUFFER, obj->getTextureCoordCount() * 2 * sizeof(GLfloat), obj->getTextureArray(), GL_STATIC_DRAW);

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
                /*char input[textureName.length() + 1];
                for (int m = 0; m < textureName.length(); ++m) input[m] = textureName[m];
                input[textureName.length()] = '\0';*/
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

    MatrixUtil::setIdentityM(mMvpMatrix, 0);

    MatrixUtil::multiplyMM(mMvpMatrix, mProjectionMatrix, mViewMatrix);
    MatrixUtil::multiplyMM(mMvpMatrix, mMvpMatrix, mModelMatrix);

    glUniformMatrix4fv(mMVPMatrixHandler, 1, false, mMvpMatrix);
    glUniformMatrix4fv(mMMatrixHandler, 1, false, mModelMatrix);
    glUniform3f(mLightLocationHandler, 3, 0 ,3);

    int size = mObjGroup->getObjSize();
    Obj3D* obj{nullptr};
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

        glDrawArrays(GL_TRIANGLES, 0, obj->getVertexCount());
    }

    glDisableVertexAttribArray(mVertexPosHandler);
    glDisableVertexAttribArray(mTextureCoordinateHandler);
    glDisableVertexAttribArray(mVertexNormalHandler);

    glBindTexture(GL_TEXTURE_2D, 0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    return inputTextureId;
}

void ModelFilter::onPause() {
    BaseFilter::onPause();
    int count = 0;
    if (mObjGroup == nullptr || (count = mObjGroup->getObjSize()) == 0) {
        LogUtil::logI(TAG, {"onPause: no obj"});
    } else {
        if (mVertexBufferId != nullptr) { glDeleteBuffers(count, mVertexBufferId); }
        if (mTextureCoordBufferId != nullptr) { glDeleteBuffers(count, mTextureCoordBufferId); }
        if (mVertexNormalBufferId != nullptr) { glDeleteBuffers(count, mVertexNormalBufferId); }

        delete mVertexBufferId;
        delete mTextureCoordBufferId;
        delete mVertexNormalBufferId;

        mVertexBufferId = nullptr;
        mTextureCoordBufferId = nullptr;
        mVertexNormalBufferId = nullptr;

        if (mModelTexture != nullptr) { glDeleteTextures(count, mModelTexture); }
        delete mModelTexture;
        mModelTexture = nullptr;
    }
}

void ModelFilter::onResume() {
    BaseFilter::onResume();
    init();
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

void ModelFilter::setOutputSize(GLint width, GLint height) {
    BaseFilter::setOutputSize(width, height);
    float ratio = width >= height ? (float)width / height : (float)height / width;
    LogUtil::logI(TAG, {"setOutputSize: width = ", std::to_string(width), ", height = ", std::to_string(height),
                        ", ratio = ", std::to_string(ratio)});
    if (width >= height) {
        MatrixUtil::orthogonal(mProjectionMatrix, 0, -ratio, ratio, -1, 1, 0, 5);
    } else {
        MatrixUtil::orthogonal(mProjectionMatrix, 0, -1, 1, -ratio, ratio, 0, 5);
    }
    MatrixUtil::setLookAt(mViewMatrix, 0, 0, 0, 5, 0, 0, 0, 0, 1, 0);

    MatrixUtil::scaleM(mModelMatrix, 0, 0.2, 0.2, 0.2);
    MatrixUtil::translateM(mModelMatrix, 0, 0, -0.3, 0.2);
}



