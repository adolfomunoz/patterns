#pragma once

#include "shape.h"
#include <cmath>

class Circle : public pattern::Reflectable<Circle,Shape> {
    float radius_;
public:
    Circle(float radius = 1.0f) : radius_(radius) {}
    float radius() const noexcept { return radius_; }
    float area() const override { return 3.14159276f*radius()*radius(); }
    static const char* type_name() { return "circle"; }
    auto reflect_names() const { return std::tuple("radius"); }
    auto reflect() { return std::tie(radius_);  }
};

