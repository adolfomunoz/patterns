#pragma once

#include "shape.h"
#include <cmath>

class Circle : public pattern::Reflectable<Circle,ShapeBase> {
    float radius;
    float area_;
public:
    void init() { area_ = 3.14159276f*radius*radius; }
    Circle(float radius = 1.0f) : radius(radius) { init(); }
    float area() const override { return area_; }
    static const char* type_name() { return "circle"; }
    auto reflect_names() const { return std::tuple("radius"); }
    auto reflect() { return std::tie(radius);  }
};

