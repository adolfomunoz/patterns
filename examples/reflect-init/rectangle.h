#pragma once

#include "../../patterns.h"

class Rectangle : public pattern::Reflectable<Rectangle> {
    float width,height;
    float area_;

public:
    void init() { area_ = width*height; } //This init method is for updating after a load and construction.
    Rectangle(float width = 1.0f, float height = 1.0f) : width(width), height(height) { init(); }
    float area() const { return area_; }
    auto reflect() { return std::tie(width,height); }
    auto reflect_names() const { return std::tuple("width","height"); }
    static const char* type_name() { return "rectangle"; }
};
