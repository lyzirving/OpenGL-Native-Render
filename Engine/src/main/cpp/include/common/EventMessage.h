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
    EVENT_FACE_LAND_MARK_TRACK = 7,
    EVENT_FACE_TRACK_START = 8,
    EVENT_FACE_TRACK_STOP = 9,
    EVENT_CHANGE_STATUS = 10,
    EVENT_ADD_SHARE_CONTEXT = 11,
    EVENT_DRAW_SHARE_ENV = 12,
    EVENT_SET_CLIENT = 13,
    EVENT_QUIT = 14
};

class EventMessage {
public:
    EventMessage();
    EventMessage(EventType msg);
    EventMessage(EventType msg, int inputArg0);
    EventMessage(EventType msg, int inputArg0, int inputArg1);
    EventMessage(const EventMessage& eventMessage) noexcept;
    EventMessage(EventMessage&& eventMessage) noexcept;
    EventType what;
    int arg0{-1};
    int arg1{-1};
private:
};

#endif //ENGINE_EVENTMESSAGE_H
