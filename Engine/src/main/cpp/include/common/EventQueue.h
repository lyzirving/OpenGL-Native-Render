//
// Created by liuyuzhou on 2021/9/7.
//
#ifndef ENGINE_EVENTQUEUE_H
#define ENGINE_EVENTQUEUE_H
#include <pthread.h>
#include <queue>

enum class EventType : int8_t {
    EVENT_IDLE = 0,
    EVENT_DRAW = 1,
    EVENT_SURFACE_CHANGE = 2,
    EVENT_PAUSE = 3,
    EVENT_RESUME = 4,
    EVENT_BUILD_OES_TEXTURE = 5,
    EVENT_QUIT = 6
};

class EventMessage {
public:
    EventMessage() {
        what = EventType::EVENT_IDLE;
    }
    EventMessage(EventType msg) {
        what = msg;
    }
    EventMessage(const EventMessage& message) noexcept {
        what = message.what;
    }
    EventMessage(EventMessage&& message) noexcept {
        what = message.what;
        message.what = EventType::EVENT_IDLE;
    }
    ~EventMessage() {}
    EventType what;
private:
};

class EventQueue {
public:
    EventQueue() {
        mMessageQueue = new std::queue<EventMessage>();
        pthread_mutex_init(&mMutex, nullptr);
        pthread_cond_init(&mCond, nullptr);
    }
    ~EventQueue() {
        clear();
        pthread_mutex_destroy(&mMutex);
        pthread_cond_destroy(&mCond);
        if (mMessageQueue != nullptr) { delete mMessageQueue; }
        mMessageQueue = nullptr;
    }

    void clear();
    void enqueueMessage(EventMessage* message);
    EventMessage dequeueMessage();
private:
    pthread_mutex_t mMutex;
    pthread_cond_t mCond;
    std::queue<EventMessage>* mMessageQueue;
};

#endif //ENGINE_EVENTQUEUE_H
