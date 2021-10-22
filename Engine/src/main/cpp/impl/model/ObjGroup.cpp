//
// Created by liuyuzhou on 2021/10/22.
//
#include "ObjGroup.h"

ObjGroup::ObjGroup() {
    mObjs = new std::vector<Obj3D*>;
}

ObjGroup::~ObjGroup() {
    Obj3D* obj{nullptr};
    for (int i = 0; i < mObjs->size(); ++i) {
        obj = mObjs->at(i);
        delete obj;
    }
    delete mObjs;
    delete mMtlLib;
}

Obj3D* ObjGroup::getObj(int index) { return mObjs->at(index); }

void ObjGroup::pushBackObj(Obj3D *obj) { mObjs->push_back(obj); }

void ObjGroup::setMtlLib(MtlLib *lib) { mMtlLib = lib; }

