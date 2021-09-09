#pragma once

#include "shape.h"
#include <cmath>

class Circle : public ShapeBase {
    float radius;
public:
    Circle(float radius = 1.0f) : radius(radius) {}
    float area() const override { return 3.14159276f*radius*radius; }
};

