#pragma once

#include "shape.h"
#include <cmath>

class Rectangle: public pattern::Reflectable<Rectangle,ShapeBase> {
    float width,height;
    float area_;
public:
    void init() { area_ = width*height; }
    Rectangle(float width = 1.0f, float height = 1.0f) : width(width), height(height) { init(); }
    float area() const override { return area_; }
    static const char* type_name() { return "rectangle"; }
    auto reflect_names() const { return std::tuple("width","height"); }
    auto reflect() { return std::tie(width,height); } 
};

