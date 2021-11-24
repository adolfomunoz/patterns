#pragma once

#include "../../patterns.h"

class Rectangle : public pattern::Reflectable<Rectangle> {
    float width,height;
public:
    Rectangle(float width = 1.0f, float height = 1.0f) : width(width), height(height) {}
    float area() const { return width*height; }
    auto reflect() { return std::tie(width,height); }
    auto reflect_names() const { return std::tuple("width","height"); }
    static const char* type_name() { return "rectangle"; }
};
