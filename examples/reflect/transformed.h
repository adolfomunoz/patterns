#pragma once

#include "rectangle.h"

class Transformed : public pattern::Reflectable<Transformed> {
    float x, y;
    Rectangle rect;
public:
    Transformed(float x, float y, const Rectangle& r) : x(x), y(y), rect(r) {}
    auto reflect() { return std::tie(x,y,rect); }
    auto reflect_names() const { return std::tuple("x","y"); }
    static const char* type_name() { return "transformed"; }
};
