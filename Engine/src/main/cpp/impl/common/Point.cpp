//
// Created by liuyuzhou on 2021/10/8.
//
#include "Point.h"

Point::Point() {
    x = 0;
    y = 0;
}

Point::Point(float inputX, float inputY) {
    x = inputX;
    y = inputY;
}

Point::Point(const Point &pt) noexcept {
    x = pt.x;
    y = pt.y;
}

Point::Point(Point &&pt) noexcept {
    x = pt.x;
    y = pt.y;
}

Point& Point::operator=(const Point &pt) {
    if(this != &pt) {
        this->x = pt.x;
        this->y = pt.y;
    }
    return *this;
}

