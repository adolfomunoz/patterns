#pragma once

#include "shape.h"
#include <cmath>

class Circle : public pattern::SelfRegisteringReflectable<Circle,ShapeBase> {
    float radius;
public:
    Circle(float radius = 1.0f) : radius(radius) {}
    float area() const override { return 3.14159276f*radius*radius; }
    static const char* type_name() { return "circle"; }
    auto reflect_names() const { return std::tuple("radius"); }
    auto reflect() { return std::tie(radius);  }
};

