//
// Created by liuyuzhou on 2021/10/20.
//
#include <string>
#include <vector>
#include <unistd.h>
#include <sys/stat.h>
#include <fstream>

#include "ModelLoader.h"
#include "GlUtil.h"
#include "LogUtil.h"

#define TAG "ModelLoader"
#define MODEL_ROOT_PATH "/storage/emulated/0/Android/data/com.render.demo/files/Documents/Model"
#define ASSETS_MODEL_ROOT "obj"

bool ModelLoader::buildLocalSource(ResourceType type, const char *modelName) {
    bool res{false};
    char* assetsContent = nullptr;
    FILE *assetsCopyFile = nullptr;
    char assetsPath[256] = {0};
    char assetsLocalDir[256] = {0};
    int assetsLen{0};
    if (access(MODEL_ROOT_PATH, 0) == -1) { mkdir(MODEL_ROOT_PATH, 0666); }
    if (modelName == nullptr || std::strlen(modelName) == 0) {
        LogUtil::logI(TAG, {"loadObj: model name is invalid"});
        res = false;
        goto quit;
    }
    switch(type) {
        case ResourceType::OBJ: {
            sprintf(assetsLocalDir, "%s/%s_obj", MODEL_ROOT_PATH, modelName);
            sprintf(assetsPath, "%s/%s_obj", ASSETS_MODEL_ROOT, modelName);
            break;
        }
        case ResourceType::MTL: {
            sprintf(assetsLocalDir, "%s/%s_mtl", MODEL_ROOT_PATH, modelName);
            sprintf(assetsPath, "%s/%s_mtl", ASSETS_MODEL_ROOT, modelName);
            break;
        }
        default: {
            LogUtil::logI(TAG, {"loadObj: invalid type: ", std::to_string((u_int8_t)type)});
            res = false;
            goto quit;
        }
    }

    if (access(assetsLocalDir, 0) != -1) {
        LogUtil::logI(TAG, {"loadObj: file already exists: ", assetsLocalDir});
    } else {
        assetsContent = GlUtil::self()->readAssets(assetsPath, &assetsLen);
        if (assetsContent == nullptr || std::strlen(assetsContent) == 0 || assetsLen == 0) {
            LogUtil::logI(TAG, {"loadObj: assets content is invalid"});
            res = false;
            goto quit;
        }
        assetsCopyFile = fopen(assetsLocalDir, "wb");
        fwrite(assetsContent, 1, assetsLen, assetsCopyFile);
        LogUtil::logI(TAG, {"loadObj: copy assets to local directory"});
    }
    res = true;
    goto quit;
    quit:
    if (assetsContent != nullptr) free(assetsContent);
    if (assetsCopyFile != nullptr) fclose(assetsCopyFile);
    return res;
}

ObjGroup* ModelLoader::buildModel(const char *modelName) {
    char srcPath[256] = {0};
    sprintf(srcPath, "%s/%s_obj", MODEL_ROOT_PATH, modelName);
    std::ifstream input(srcPath);
    std::string line;
    ObjGroup* objGroup{nullptr};
    if (input) {
        objGroup = new ObjGroup;
        std::string pattern(" ");
        std::vector<std::string> subStr;

        std::vector<float> vertex;
        std::vector<float> textureCoordinate;
        std::vector<float> vertexNormal;
        std::vector<MtlLib*> mtlLibs;
        MtlObj* mtlObj{nullptr};
        //remember the MtlObj should be explicitly deconstructed
        std::vector<MtlObj*> mtlGroup;

        while(getline(input, line)) {
            if (line.empty() || line[0] == '#') { continue; }

            split(line, pattern, subStr);
            if (subStr.empty()) { continue; }
            std::string type = subStr[0];

            if ("mtllib" == type) {
                std::string::size_type pos = subStr[1].find(".mtl");
                if (pos == std::string::npos) {
                    LogUtil::logI(TAG, {"buildModel: no mtl find"});
                    continue;
                } else {
                    std::string mtlLibName(subStr[1].substr(0, pos));
                    LogUtil::logI(TAG, {"buildModel: mtl lib name = ", mtlLibName});
                    mtlLibs.push_back(parseMtlLib(mtlLibName));
                }
            } else if ("v" == type) {
                vertex.push_back(transToFloat(subStr[1]));
                vertex.push_back(transToFloat(subStr[2]));
                vertex.push_back(transToFloat(subStr[3]));
            } else if ("vt" == type) {
                textureCoordinate.push_back(transToFloat(subStr[1]));
                textureCoordinate.push_back(transToFloat(subStr[2]));
            } else if ("vn" == type) {
                vertexNormal.push_back(transToFloat(subStr[1]));
                vertexNormal.push_back(transToFloat(subStr[2]));
                vertexNormal.push_back(transToFloat(subStr[3]));
            } else if ("usemtl" == type) {
                mtlObj = new MtlObj;
                mtlObj->setMtlSrcName(subStr[1]);
                mtlGroup.push_back(mtlObj);
            } else if ("f" == type) {
                if (mtlObj == nullptr) { continue; }
                std::vector<std::string> faceData;
                std::string faceSeparator("/");
                int idx;
                for (int i = 1; i < subStr.size(); ++i) {
                    split(subStr[i], faceSeparator, faceData);

                    idx = transToInt(faceData[0]);
                    if (idx < 0)
                        idx = (int)vertex.size() / 3 + idx;
                    else
                        idx -=1;
                    mtlObj->appendVertexIndex(idx);

                    idx = transToInt(faceData[1]);
                    if (idx < 0)
                        idx = (int)textureCoordinate.size() / 2 + idx;
                    else
                        idx -=1;
                    mtlObj->appendTextureIndex(idx);

                    idx = transToInt(faceData[2]);
                    if (idx < 0)
                        idx = (int)vertexNormal.size() / 3 + idx;
                    else
                        idx -=1;
                    mtlObj->appendVertexNormalIndex(idx);
                }
            }
        }
        for (int i = 0; i < mtlGroup.size(); ++i) {
            mtlObj = mtlGroup[i];
            auto* obj = new Obj3D;
            int vertexIndCount = mtlObj->vertexIndexCount();
            int textureIndCount = mtlObj->textureIndexCount();
            int vertexNormalCount = mtlObj->vertexNormalIndexCount();
            int index;

            obj->buildVertexArray(vertexIndCount);
            obj->buildTextureArray(textureIndCount);
            obj->buildVertexNormalArray(vertexNormalCount);
            obj->setMtlSrc(mtlObj->getMtlSrcName());

            for (int j = 0; j < vertexIndCount; ++j) {
                index = mtlObj->getVertexIndex(j);
                obj->setVertexArray(3 * j, vertex.at(3 * index));
                obj->setVertexArray(3 * j + 1, vertex.at(3 * index + 1));
                obj->setVertexArray(3 * j + 2, vertex.at(3 * index + 2));
            }
            for (int j = 0; j < textureIndCount; ++j) {
                index = mtlObj->getTextureIndex(j);
                obj->setTextureArray(2 * j, textureCoordinate.at(2 * index));
                obj->setTextureArray(2 * j + 1, textureCoordinate.at(2 * index + 1));
            }
            for (int j = 0; j < vertexNormalCount; ++j) {
                index = mtlObj->getVertexNormalIndex(j);
                obj->setVertexNormalArray(3 * j, vertexNormal.at(3 * index));
                obj->setVertexNormalArray(3 * j + 1, vertexNormal.at(3 * index + 1));
                obj->setVertexNormalArray(3 * j + 2, vertexNormal.at(3 * index + 2));
            }
            for (auto tmpLib : mtlLibs) {
                if (tmpLib->getNewMtl() == obj->getMtlSource()) {
                    obj->setMtlLib(tmpLib);
                }
            }
            objGroup->pushBackObj(obj);
            delete mtlObj;
        }
        mtlGroup.clear();
        LogUtil::logI(TAG, {"buildModel: obj size = ", std::to_string(objGroup->getObjSize())});
    }
    input.clear();
    input.close();
    return objGroup;
}

MtlLib* ModelLoader::parseMtlLib(const std::string &name) {
    char srcPath[256] = {0};
    sprintf(srcPath, "%s/%s_mtl", MODEL_ROOT_PATH, name.c_str());
    std::ifstream input(srcPath);
    std::string line;
    MtlLib* lib{nullptr};
    if (input) {
        std::string pattern(" ");
        std::vector<std::string> subStr;
        lib = new MtlLib;

        while(getline(input, line)) {
            if (line.empty() || line[0] == '#') { continue; }

            split(line, pattern, subStr);
            if (subStr.empty()) { continue; }
            std::string type = subStr[0];

            if ("newmtl" == type) {
                lib->setNewMtl(subStr[1]);
            } else if ("Ns" == type) {
                lib->setNs(transToFloat(subStr[1]));
            } else if ("Ka" == type) {
                lib->setKa(transToFloat(subStr[1]), transToFloat(subStr[2]), transToFloat(subStr[3]));
            } else if ("Kd" == type) {
                lib->setKd(transToFloat(subStr[1]), transToFloat(subStr[2]), transToFloat(subStr[3]));
            } else if("Ks" == type) {
                lib->setKs(transToFloat(subStr[1]), transToFloat(subStr[2]), transToFloat(subStr[3]));
            } else if("Ke" == type) {
                lib->setKe(transToFloat(subStr[1]), transToFloat(subStr[2]), transToFloat(subStr[3]));
            } else if ("Ni" == type) {
                lib->setNi(transToFloat(subStr[1]));
            } else if ("d" == type) {
                lib->setD(transToFloat(subStr[1]));
            } else if ("illum" == type) {
                lib->setIllm(transToInt(subStr[1]));
            } else if ("map_Kd" == type) {
                lib->setMapKd(subStr[1]);
            }
        }
    }
    input.clear();
    input.close();
    return lib;
}

void ModelLoader::split(const std::string &src, std::string &pattern,
                        std::vector<std::string> &result) {
    result.clear();
    std::string original(src);
    original += pattern;
    int size = original.size();
    std::string::size_type pos;
    for (int i = 0; i < size; ++i) {
        pos = original.find(pattern, i);
        if (pos < size) {
            result.push_back(std::move(original.substr(i, pos - i)));
            i = pos + pattern.size() - 1;
        }
    }
}

double ModelLoader::transToDouble(const std::string &src) {
    return strtod(src.c_str(), nullptr);
}

float ModelLoader::transToFloat(const std::string &src) {
    return strtof(src.c_str(), nullptr);
}

int ModelLoader::transToInt(const std::string &src) {
    return std::stoi(src);
}



