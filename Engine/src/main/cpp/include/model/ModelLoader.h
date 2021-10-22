//
// Created by liuyuzhou on 2021/10/20.
//
#ifndef RENDERDEMO_MODELLOADER_H
#define RENDERDEMO_MODELLOADER_H

#include "ObjGroup.h"
#include "MtlObj.h"

/**
 * todo: 修改加载的数据类型 double -> float
 */

enum class ResourceType : u_int8_t {
    OBJ = 1,
    MTL = 2
};

class ModelLoader {
public:
    bool buildLocalSource(ResourceType type, const char *modelName);

    void loadObj(const char *modelName);

private:
    ObjGroup *buildModel(const char *modelName);

    MtlLib *parseMtlLib(const std::string &name);

    void split(const std::string &src, std::string &pattern, std::vector<std::string> &result);

    double transToDouble(const std::string &src);

    int transToInt(const std::string &src);
};

#endif //RENDERDEMO_MODELLOADER_H
