//
// Created by lyzirving on 2021/9/27.
//
#include "EventMessage.h"

EventMessage::EventMessage() {
    what = EventType::EVENT_IDLE;
}

EventMessage::EventMessage(EventType type) {
    what = type;
}

EventMessage::EventMessage(EventType type, int inputArg0) {
    what = type;
    arg0 = inputArg0;
}

EventMessage::EventMessage(const EventMessage &eventMessage) noexcept {
    what = eventMessage.what;
    arg0 = eventMessage.arg0;
}

EventMessage::EventMessage(EventMessage &&eventMessage) noexcept {
    what = eventMessage.what;
    arg0 = eventMessage.arg0;
}
