//
// Created by liuyuzhou on 2021/10/22.
//
#ifndef ENGINE_OBJGROUP_H
#define ENGINE_OBJGROUP_H

#include "Obj3D.h"
#include "MtlLib.h"

class ObjGroup {
public:
    ObjGroup();
    ~ObjGroup();

    Obj3D* getObj(int index);
    void setMtlLib(MtlLib* lib);
    void pushBackObj(Obj3D* obj);
private:
    std::vector<Obj3D*>* mObjs;
    MtlLib* mMtlLib{nullptr};
};

#endif //ENGINE_OBJGROUP_H
