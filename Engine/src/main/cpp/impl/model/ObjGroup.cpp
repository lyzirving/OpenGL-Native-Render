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
}

void ObjGroup::clearObj() {
    while(mObjs != nullptr && !mObjs->empty()) {
        auto iterator = mObjs->end();
        mObjs->erase(--iterator);
        delete *iterator;
    }
}

Obj3D* ObjGroup::getObj(int index) { return mObjs->at(index); }

int ObjGroup::getObjSize() {
    if (mObjs == nullptr || mObjs->empty()) {
        return 0;
    } else {
        return mObjs->size();
    }
}

void ObjGroup::pushBackObj(Obj3D *obj) { mObjs->push_back(obj); }

