//
// Created by liuyuzhou on 2021/10/8.
//
#ifndef ENGINE_POINT_H
#define ENGINE_POINT_H

class Point {
public:
    float x{0};
    float y{0};

    Point();
    Point(float inputX, float inputY);
    Point(const Point& pt) noexcept;
    Point(Point&& pt) noexcept;

    Point& operator=(const Point& pt);
};

#endif //ENGINE_POINT_H
