//
// Created by lyzirving on 2021/9/27.
//
#ifndef ENGINE_CUSTOMQUEUE_H
#define ENGINE_CUSTOMQUEUE_H

#include <pthread.h>
#include <queue>

template <typename T>
class CustomQueue {
public:
    CustomQueue() {
        pthread_mutex_init(&mMutex, nullptr);
        pthread_cond_init(&mCond, nullptr);
        mQueue = new std::queue<T>;
    }
    ~CustomQueue() {
        while (!mQueue->empty()) mQueue->pop();
        delete mQueue;
        mQueue = nullptr;
        pthread_mutex_destroy(&mMutex);
        pthread_cond_destroy(&mCond);
    }

    void clear() {
        pthread_mutex_lock(&mMutex);
        while (!mQueue->empty()) mQueue->pop();
        delete mQueue;
        mQueue = new std::queue<T>;
        pthread_mutex_unlock(&mMutex);
    }

    void enqueue(T&& obj) {
        pthread_mutex_lock(&mMutex);
        mQueue->push(std::forward<T>(obj));
        pthread_cond_signal(&mCond);
        pthread_mutex_unlock(&mMutex);
    }

    T dequeue() {
        pthread_mutex_lock(&mMutex);
        if (mQueue->empty()) { pthread_cond_wait(&mCond, &mMutex); }

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

private:
    pthread_mutex_t mMutex{};
    pthread_cond_t mCond{};
    std::queue<T>* mQueue{nullptr};
};

#endif //ENGINE_CUSTOMQUEUE_H
