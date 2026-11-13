#pragma once

#include "shape.h"
#include <cmath>

class Rectangle: public ShapeBase {
    float width_,height_;
public:
    Rectangle(float width = 1.0f, float height = 1.0f) : width_(width), height_(height) {}
    float width() const { return width_; }
    float height() const { return height_; }
    float area() const override { return width_*height_; }
};

