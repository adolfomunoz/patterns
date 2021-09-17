#pragma once

#include "../../src/reflect.h"

class Rectangle : public pattern::Reflectable<Rectangle> {
    float width,height;
public:
    Rectangle(float width = 1.0f, float height = 1.0f) : width(width), height(height) {}
    float area() const { return width*height; }
    auto reflect() const { return std::tie(width,height); }
};
