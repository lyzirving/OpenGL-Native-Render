//
// Created by liuyuzhou on 2021/10/21.
//
#include "MtlObj.h"
#include "LogUtil.h"

#define TAG "MtlObj"

MtlObj::MtlObj() {
    mVertexIndex = new std::vector<int>;
    mTextureCoordinateIndex = new std::vector<int>;
    mVertexNormalIndex = new std::vector<int>;
}

MtlObj::~MtlObj() {
    LogUtil::logI(TAG, {"deconstruct"});
    mVertexIndex->clear();
    mTextureCoordinateIndex->clear();
    mVertexNormalIndex->clear();
    delete mVertexIndex;
    delete mTextureCoordinateIndex;
    delete mVertexNormalIndex;
}

void MtlObj::appendVertexIndex(int index) { mVertexIndex->push_back(index); }

void MtlObj::appendTextureIndex(int index) { mTextureCoordinateIndex->push_back(index); }

void MtlObj::appendVertexNormalIndex(int index) { mVertexNormalIndex->push_back(index); }

int MtlObj::getVertexIndex(int index) { return mVertexIndex->at(index); }

int MtlObj::getTextureIndex(int index) { return mTextureCoordinateIndex->at(index); }

int MtlObj::getVertexNormalIndex(int index) { return mVertexNormalIndex->at(index); }

std::string MtlObj::getMtlSrcName() { return mMtlSrcName; }

void MtlObj::setMtlSrcName(std::string &name) { mMtlSrcName = name; }

int MtlObj::vertexIndexCount() { return mVertexIndex->size(); }

int MtlObj::textureIndexCount() { return mTextureCoordinateIndex->size(); }

int MtlObj::vertexNormalIndexCount() { return mVertexNormalIndex->size(); }

