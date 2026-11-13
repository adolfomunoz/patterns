#pragma once

#include "shape.h"
#include <cmath>

class Circle : public ShapeBase {
    float radius_;
public:
    Circle(float radius = 1.0f) : radius_(radius) {}
    float radius() const { return radius_; }
    float area() const override { return 3.14159276f*radius_*radius_; }
};

