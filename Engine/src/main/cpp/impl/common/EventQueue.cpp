//
// Created by liuyuzhou on 2021/9/7.
//
#include "EventQueue.h"

#define TAG "EventQueue"

void EventQueue::clear() {
    pthread_mutex_lock(&mMutex);
    while (!mMessageQueue->empty()) mMessageQueue->pop();
    pthread_mutex_unlock(&mMutex);
}

EventMessage EventQueue::dequeueMessage() {
    pthread_mutex_lock(&mMutex);
    if (mMessageQueue->empty()) {
        pthread_cond_wait(&mCond, &mMutex);
    }
    //copy construct
    EventMessage result = mMessageQueue->front();
    mMessageQueue->pop();
    pthread_mutex_unlock(&mMutex);
    return std::move(result);
}

void EventQueue::enqueueMessage(EventMessage *message) {
    pthread_mutex_lock(&mMutex);
    mMessageQueue->push(*message);
    pthread_cond_signal(&mCond);
    pthread_mutex_unlock(&mMutex);
}

