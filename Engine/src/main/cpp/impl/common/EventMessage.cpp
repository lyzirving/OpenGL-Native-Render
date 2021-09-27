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

EventMessage::EventMessage(const EventMessage &eventMessage) noexcept {
    what = eventMessage.what;
}

EventMessage::EventMessage(EventMessage &&eventMessage) noexcept {
    what = eventMessage.what;
}
