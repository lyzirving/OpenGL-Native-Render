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
        if (mQueue->empty()) {
            if (mDebug) { LogUtil::logI(mTag, {"dequeue: start to wait"}); }
            pthread_cond_wait(&mCond, &mMutex);
        }
        if (mQueue->empty()) {
            if (mDebug) { LogUtil::logI(mTag, {"dequeue: empty, return new object"}); }
            pthread_mutex_unlock(&mMutex);
            return T();
        }
        T result = mQueue->front();
        mQueue->pop();
        pthread_mutex_unlock(&mMutex);
        /**
         * the compiler will execute RVO
         * because local filed is returned,
         * and the type of filed equals to the return type
         */
        if (mDebug) { LogUtil::logI(mTag, {"dequeue: front and pop"}); }
        return result;
    }

    T dequeueNotWait() {
        pthread_mutex_lock(&mMutex);
        if (mQueue->empty()) {
            pthread_mutex_unlock(&mMutex);
            return T();
        }
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

    void setDebug(bool debug) { mDebug = debug; }
    void setTag(std::string&& tag) { mTag = std::move(tag); }

private:
    pthread_mutex_t mMutex{};
    pthread_cond_t mCond{};
    std::queue<T>* mQueue{nullptr};
    std::string mTag{"ObjectQueue"};
    bool mDebug{false};
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
        if (mPointerQueue->empty()) {
            if (mDebug) { LogUtil::logI(mTag, {"dequeue: wait"}); }
            pthread_cond_wait(&mCond, &mMutex);
        }
        if (mPointerQueue->empty()) {
            if (mDebug) { LogUtil::logI(mTag, {"dequeue: return new obj"}); }
            pthread_mutex_unlock(&mMutex);
            return PointerType();
        }
        PointerType result = mPointerQueue->front();
        mPointerQueue->pop();
        pthread_mutex_unlock(&mMutex);
        if (mDebug) { LogUtil::logI(mTag, {"dequeue: front and pop"}); }
        return result;
    }

    PointerType dequeueNotWait() {
        pthread_mutex_lock(&mMutex);
        if (mPointerQueue->empty()) {
            if (mDebug) { LogUtil::logI(mTag, {"dequeueNotWait: empty, return null"}); }
            pthread_mutex_unlock(&mMutex);
            return nullptr;
        }
        PointerType result = mPointerQueue->front();
        mPointerQueue->pop();
        pthread_mutex_unlock(&mMutex);
        if (mDebug) { LogUtil::logI(mTag, {"dequeueNotWait: return gained obj"}); }
        //execute RVO
        return result;
    }

    void notify() {
        pthread_mutex_lock(&mMutex);
        pthread_cond_signal(&mCond);
        pthread_mutex_unlock(&mMutex);
    }

    void setDebug(bool debug) { mDebug = debug; }
    void setTag(std::string&& tag) { mTag = std::move(tag); }

private:
    pthread_mutex_t mMutex{};
    pthread_cond_t mCond{};
    std::queue<PointerType>* mPointerQueue{nullptr};
    std::string mTag{"PointerQueue"};
    bool mDebug{false};
};

#endif //ENGINE_CUSTOMQUEUE_H
