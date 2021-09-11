//
// Created by liuyuzhou on 2021/9/6.
//
#include "WorkQueue.h"

void WorkQueue::clear() {
    pthread_mutex_lock(&mMutex);
    while (!mQueue->empty()) mQueue->pop();
    pthread_mutex_unlock(&mMutex);
}

bool WorkQueue::dequeue(std::shared_ptr<WorkTask> &task) {
    pthread_mutex_lock(&mMutex);
    if (mQueue->empty()) {
        pthread_mutex_unlock(&mMutex);
        return false;
    }
    task = mQueue->front();
    mQueue->pop();
    pthread_mutex_unlock(&mMutex);
    return true;
}

void WorkQueue::enqueue(const std::shared_ptr<WorkTask>& task) {
    pthread_mutex_lock(&mMutex);
    mQueue->push(task);
    pthread_mutex_unlock(&mMutex);
}

bool WorkQueue::empty() {
    return mQueue != nullptr ? mQueue->empty() : true;
}