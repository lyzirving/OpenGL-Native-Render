//
// Created by liuyuzhou on 2021/10/21.
//
#ifndef ENGINE_MTLOBJ_H
#define ENGINE_MTLOBJ_H

#include <vector>
#include <string>

class MtlObj {
public:

    MtlObj();

    ~MtlObj();

    void appendVertexIndex(int index);
    void appendTextureIndex(int index);
    void appendVertexNormalIndex(int index);
    void setMtlSrcName(std::string& name);

    int getVertexIndex(int index);
    int getTextureIndex(int index);
    int getVertexNormalIndex(int index);
    std::string getMtlSrcName();

    int vertexIndexCount();
    int textureIndexCount();
    int vertexNormalIndexCount();

private:
    std::vector<int> *mVertexIndex{nullptr};
    std::vector<int> *mTextureCoordinateIndex{nullptr};
    std::vector<int> *mVertexNormalIndex{nullptr};
    std::string mMtlSrcName;
};

#endif //ENGINE_MTLOBJ_H
