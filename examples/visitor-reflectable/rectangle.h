#pragma once

#include "shape.h"
#include <cmath>

class Rectangle: public pattern::Reflectable<Rectangle,Shape> {
    float width_,height_;
public:
    Rectangle(float width = 1.0f, float height = 1.0f) : width_(width), height_(height) {}
    float width() const noexcept { return width_; }
    float height() const noexcept { return height_; }
    float area() const override { return width()*height(); }
    static const char* type_name() { return "rectangle"; }
    auto reflect_names() const { return std::tuple("width","height"); }
    auto reflect() { return std::tie(width_,height_); } 
};

