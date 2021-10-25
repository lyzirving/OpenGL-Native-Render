//
// Created by liuyuzhou on 2021/10/22.
//
#ifndef ENGINE_OBJGROUP_H
#define ENGINE_OBJGROUP_H

#include "Obj3D.h"

class ObjGroup {
public:
    ObjGroup();
    ~ObjGroup();

    Obj3D* getObj(int index);
    int getObjSize();
    void pushBackObj(Obj3D* obj);
private:
    std::vector<Obj3D*>* mObjs;
};

#endif //ENGINE_OBJGROUP_H
