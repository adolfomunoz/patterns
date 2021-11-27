#pragma once

#include "shape.h"
#include <cmath>

class Rectangle: public pattern::SelfRegisteringReflectable<Rectangle,ShapeBase> {
    float _width,_height;
public:
    Rectangle(float width = 1.0f, float height = 1.0f) : _width(width), _height(height) {}
    float width() const { return _width; }
    float height() const { return _height; }
    float area() const override { return width()*height(); }
    static const char* type_name() { return "rectangle"; }
    auto reflect_names() const { return std::tuple("width","height"); }
    auto reflect() { return std::tie(_width,_height); } 
};

