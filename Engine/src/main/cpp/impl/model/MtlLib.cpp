//
// Created by liuyuzhou on 2021/10/22.
//
#include "MtlLib.h"

void MtlLib::setD(double d) { mD = d; }

void MtlLib::setNi(double ni) { mNi = ni; }

void MtlLib::setNs(double ns) { mNs = ns; }

void MtlLib::setIllm(int illm) { mIllm = illm; }

void MtlLib::setKe(double ke1, double ke2, double ke3) {
    mKe[0] = ke1;
    mKe[1] = ke2;
    mKe[2] = ke3;
}

void MtlLib::setKa(double ka1, double ka2, double ka3) {
    mKa[0] = ka1;
    mKa[1] = ka2;
    mKa[2] = ka3;
}

void MtlLib::setKd(double kd1, double kd2, double kd3) {
    mKd[0] = kd1;
    mKd[1] = kd2;
    mKd[2] = kd3;
}

void MtlLib::setKs(double ks1, double ks2, double ks3) {
    mKs[0] = ks1;
    mKs[1] = ks2;
    mKs[2] = ks3;
}

void MtlLib::setNewMtl(const std::string &newMtl) { mNewMtl = newMtl; }

void MtlLib::setMapKd(const std::string &mapKd) { mMapKd = mapKd; }

