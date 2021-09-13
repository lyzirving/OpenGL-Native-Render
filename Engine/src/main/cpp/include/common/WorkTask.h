//
// Created by liuyuzhou on 2021/9/7.
//

#ifndef ENGINE_WORKTASK_H
#define ENGINE_WORKTASK_H

class WorkTask {
public:
    virtual void run();
    ~WorkTask() { mObj = nullptr; }

    void setObj(void* obj) { mObj = obj; }
protected:
    void* mObj;
private:
};

class FilterInitTask : public WorkTask {
public:
    virtual void run() override;
private:
};

class FilterDestroyTask : public WorkTask {
public:
    virtual void run() override;
private:
};

#endif //ENGINE_WORKTASK_H
