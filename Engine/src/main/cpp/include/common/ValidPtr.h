//
// Created by lyzirving on 2021/9/30.
//
#ifndef ENGINE_OBJECTWRAPPER_H
#define ENGINE_OBJECTWRAPPER_H

template <typename T>
class ValidPtr {
public:
    ValidPtr(T* obj) {
        mPtr = obj;
        mValid = true;
    }
    bool alive() { return mValid; }
    T* get() {
        if (mValid) {
            return mPtr;
        } else {
            return nullptr;
        }
    }
    void setAlive(bool alive) { mValid = alive; }
    T* operator->() { return mPtr; }

private:
    bool mValid;
    T* mPtr{nullptr};
};

#endif //ENGINE_OBJECTWRAPPER_H
