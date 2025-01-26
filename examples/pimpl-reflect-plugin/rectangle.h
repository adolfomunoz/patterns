#pragma once

#include "shape.h"
#include <cmath>

class Rectangle: public pattern::Reflectable<Rectangle,ShapeBase> {
    float width,height;
public:
    Rectangle(float width = 1.0f, float height = 1.0f) : width(width), height(height) {}
    float area() const override { return width*height; }
    static const char* type_name() { return "rectangle"; }
    auto reflect_names() const { return std::tuple("width","height"); }
    auto reflect() { return std::tie(width,height); } 
};

