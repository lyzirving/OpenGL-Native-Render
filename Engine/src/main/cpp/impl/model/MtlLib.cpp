//
// Created by liuyuzhou on 2021/10/22.
//
#include "MtlLib.h"

float* MtlLib::getAmbient() { return mKa; }

float * MtlLib::getDiffuse() { return mKd; }

std::string MtlLib::getNewMtl() { return mNewMtl; }

std::string MtlLib::getTextureName() { return mMapKd; }

void MtlLib::setD(float d) { mD = d; }

void MtlLib::setNi(float ni) { mNi = ni; }

void MtlLib::setNs(float ns) { mNs = ns; }

void MtlLib::setIllm(int illm) { mIllm = illm; }

void MtlLib::setKe(float ke1, float ke2, float ke3) {
    mKe[0] = ke1;
    mKe[1] = ke2;
    mKe[2] = ke3;
}

void MtlLib::setKa(float ka1, float ka2, float ka3) {
    mKa[0] = ka1;
    mKa[1] = ka2;
    mKa[2] = ka3;
}

void MtlLib::setKd(float kd1, float kd2, float kd3) {
    mKd[0] = kd1;
    mKd[1] = kd2;
    mKd[2] = kd3;
}

void MtlLib::setKs(float ks1, float ks2, float ks3) {
    mKs[0] = ks1;
    mKs[1] = ks2;
    mKs[2] = ks3;
}

void MtlLib::setNewMtl(const std::string &newMtl) { mNewMtl = newMtl; }

void MtlLib::setMapKd(const std::string &mapKd) { mMapKd = mapKd; }

