//
// Created by liuyuzhou on 2021/10/22.
//
#ifndef ENGINE_MTLLIB_H
#define ENGINE_MTLLIB_H

#include <string>

class MtlLib {
public:
    std::string getNewMtl();
    std::string getTextureName();
    float* getAmbient();
    float* getDiffuse();
    void setNewMtl(const std::string& newMtl);
    void setMapKd(const std::string& mapKd);
    void setNs(float ns);
    void setNi(float ni);
    void setD(float d);
    void setKa(float ka1, float ka2, float ka3);
    void setKd(float kd1, float kd2, float kd3);
    void setKs(float ks1, float ks2, float ks3);
    void setKe(float ke1, float ke2, float ke3);
    void setIllm(int illm);
private:
    std::string mNewMtl{""};
    std::string mMapKd{""};
    int mIllm{0};
    float mNs{0};
    float mNi{0};
    float mD{0};
    float mKa[3];
    float mKd[3];
    float mKs[3];
    float mKe[3];
};

#endif //ENGINE_MTLLIB_H
