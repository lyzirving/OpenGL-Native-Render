//
// Created by lyzirving on 2021/9/27.
//
#ifndef ENGINE_EVENTMESSAGE_H
#define ENGINE_EVENTMESSAGE_H

#include <cstdint>

enum class EventType : int8_t {
    EVENT_IDLE = 0,
    EVENT_DRAW = 1,
    EVENT_SURFACE_CHANGE = 2,
    EVENT_PAUSE = 3,
    EVENT_RESUME = 4,
    EVENT_BUILD_OES_TEXTURE = 5,
    EVENT_WRITE_PNG = 6,
    EVENT_QUIT = 7
};

class EventMessage {
public:
    EventMessage();
    EventMessage(EventType msg);
    EventMessage(const EventMessage& eventMessage) noexcept;
    EventMessage(EventMessage&& eventMessage) noexcept;
    EventType what;
private:
};

#endif //ENGINE_EVENTMESSAGE_H
