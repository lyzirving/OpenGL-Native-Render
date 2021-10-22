//
// Created by liuyuzhou on 2021/10/22.
//
#ifndef ENGINE_MTLLIB_H
#define ENGINE_MTLLIB_H

#include <string>

class MtlLib {
public:
    void setNewMtl(const std::string& newMtl);
    void setMapKd(const std::string& mapKd);
    void setNs(double ns);
    void setNi(double ni);
    void setD(double d);
    void setKa(double ka1, double ka2, double ka3);
    void setKd(double kd1, double kd2, double kd3);
    void setKs(double ks1, double ks2, double ks3);
    void setKe(double ke1, double ke2, double ke3);
    void setIllm(int illm);
private:
    std::string mNewMtl{""};
    std::string mMapKd{""};
    int mIllm{0};
    double mNs{0};
    double mNi{0};
    double mD{0};
    double mKa[3];
    double mKd[3];
    double mKs[3];
    double mKe[3];
};

#endif //ENGINE_MTLLIB_H
