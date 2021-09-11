//
// Created by liuyuzhou on 2021/9/7.
//

#ifndef ENGINE_WORKQUEUE_H
#define ENGINE_WORKQUEUE_H

#include <queue>
#include <pthread.h>

#include "WorkTask.h"

class WorkQueue {
public:
    WorkQueue() {
        mQueue = new std::queue<std::shared_ptr<WorkTask>>;
        pthread_mutex_init(&mMutex, nullptr);
    }
    ~WorkQueue() {
        clear();
        delete mQueue;
        mQueue = nullptr;
        pthread_mutex_destroy(&mMutex);
    }
    void clear();
    bool dequeue(std::shared_ptr<WorkTask>& task);
    void enqueue(const std::shared_ptr<WorkTask>& task);
    bool empty();
private:
    std::queue<std::shared_ptr<WorkTask>>* mQueue;
    pthread_mutex_t mMutex;
};

#endif //ENGINE_WORKQUEUE_H
