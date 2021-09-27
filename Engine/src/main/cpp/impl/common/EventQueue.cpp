//
// Created by liuyuzhou on 2021/9/7.
//
#include "EventQueue.h"

#define TAG "EventQueue"

EventMessage::EventMessage() {
    what = EventType::EVENT_IDLE;
}

EventMessage::EventMessage(EventType type) {
    what = type;
}

EventQueue::EventQueue() {
    mMessageQueue = new std::queue<EventMessage>();
    pthread_mutex_init(&mMutex, nullptr);
    pthread_cond_init(&mCond, nullptr);
}

EventQueue::~EventQueue() {
    clear();
    pthread_mutex_destroy(&mMutex);
    pthread_cond_destroy(&mCond);
    delete mMessageQueue;
    mMessageQueue = nullptr;
}

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
    EventMessage result = mMessageQueue->front();
    mMessageQueue->pop();
    pthread_mutex_unlock(&mMutex);
    //the compiler will execute RVO
    //because local filed is returned, and the type of filed equals to the return type
    return result;
}

void EventQueue::enqueueMessage(EventMessage *message) {
    pthread_mutex_lock(&mMutex);
    mMessageQueue->push(*message);
    pthread_cond_signal(&mCond);
    pthread_mutex_unlock(&mMutex);
}

