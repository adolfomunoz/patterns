#pragma once

#include "shape.h"
#include <cmath>

class Rectangle: public ShapeBase {
    float width,height;
public:
    Rectangle(float width = 1.0f, float height = 1.0f) : width(width), height(height) {}
    float area() const override { return width*height; }
};

