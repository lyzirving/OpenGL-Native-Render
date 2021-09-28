//
// Created by lyzirving on 2021/9/27.
//
#ifndef ENGINE_CUSTOMQUEUE_H
#define ENGINE_CUSTOMQUEUE_H

#include <pthread.h>
#include <queue>
#include "LogUtil.h"

template <typename T>
class ObjectQueue {
public:
    ObjectQueue() {
        pthread_mutex_init(&mMutex, nullptr);
        pthread_cond_init(&mCond, nullptr);
        mQueue = new std::queue<T>;
    }
    ~ObjectQueue() {
        while (!mQueue->empty()) mQueue->pop();
        delete mQueue;
        mQueue = nullptr;
        pthread_mutex_destroy(&mMutex);
        pthread_cond_destroy(&mCond);
    }

    void clear() {
        pthread_mutex_lock(&mMutex);
        while (!mQueue->empty()) mQueue->pop();
        pthread_cond_signal(&mCond);
        pthread_mutex_unlock(&mMutex);
    }

    void enqueue(T&& obj) {
        pthread_mutex_lock(&mMutex);
        mQueue->push(std::move(obj));
        pthread_cond_signal(&mCond);
        pthread_mutex_unlock(&mMutex);
    }

    void enqueue(const T& obj) {
        pthread_mutex_lock(&mMutex);
        mQueue->push(obj);
        pthread_cond_signal(&mCond);
        pthread_mutex_unlock(&mMutex);
    }

    bool empty() { return mQueue != nullptr ? mQueue->empty() : true; }

    T dequeue() {
        pthread_mutex_lock(&mMutex);
        if (mQueue->empty()) { pthread_cond_wait(&mCond, &mMutex); }
        if (mQueue->empty()) { return T(); }
        T result = mQueue->front();
        mQueue->pop();
        pthread_mutex_unlock(&mMutex);
        /**
         * the compiler will execute RVO
         * because local filed is returned,
         * and the type of filed equals to the return type
         */
        return result;
    }

    T dequeueNotWait() {
        pthread_mutex_lock(&mMutex);
        if (mQueue->empty()) { return T(); }
        T result = mQueue->front();
        mQueue->pop();
        pthread_mutex_unlock(&mMutex);
        //execute RVO
        return result;
    }

    void notify() {
        pthread_mutex_lock(&mMutex);
        pthread_cond_signal(&mCond);
        pthread_mutex_unlock(&mMutex);
    }

private:
    pthread_mutex_t mMutex{};
    pthread_cond_t mCond{};
    std::queue<T>* mQueue{nullptr};
};

template <typename T>
class PointerQueue {
public:
    typedef std::shared_ptr<T> PointerType;

    PointerQueue() {
        pthread_mutex_init(&mMutex, nullptr);
        pthread_cond_init(&mCond, nullptr);
        mPointerQueue = new std::queue<PointerType>;
    }

    ~PointerQueue() {
        while (!mPointerQueue->empty()) mPointerQueue->pop();
        delete mPointerQueue;
        mPointerQueue = nullptr;
        pthread_mutex_destroy(&mMutex);
        pthread_cond_destroy(&mCond);
    }

    void clear() {
        pthread_mutex_lock(&mMutex);
        while (!mPointerQueue->empty()) mPointerQueue->pop();
        pthread_cond_signal(&mCond);
        pthread_mutex_unlock(&mMutex);
    }

    void enqueue(const PointerType& pointer) {
        pthread_mutex_lock(&mMutex);
        mPointerQueue->push(pointer);
        pthread_cond_signal(&mCond);
        pthread_mutex_unlock(&mMutex);
    }

    bool empty() { return mPointerQueue != nullptr ? mPointerQueue->empty() : true; }

    PointerType dequeue() {
        pthread_mutex_lock(&mMutex);
        if (mPointerQueue->empty()) { pthread_cond_wait(&mCond, &mMutex); }
        if (mPointerQueue->empty()) { return T(); }
        PointerType result = mPointerQueue->front();
        mPointerQueue->pop();
        pthread_mutex_unlock(&mMutex);
        return result;
    }

    PointerType dequeueNotWait() {
        pthread_mutex_lock(&mMutex);
        if (mPointerQueue->empty()) { return nullptr; }
        PointerType result = mPointerQueue->front();
        mPointerQueue->pop();
        pthread_mutex_unlock(&mMutex);
        //execute RVO
        return result;
    }

    void notify() {
        pthread_mutex_lock(&mMutex);
        pthread_cond_signal(&mCond);
        pthread_mutex_unlock(&mMutex);
    }

private:
    pthread_mutex_t mMutex{};
    pthread_cond_t mCond{};
    std::queue<PointerType>* mPointerQueue{nullptr};
};

#endif //ENGINE_CUSTOMQUEUE_H
