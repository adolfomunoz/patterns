#pragma once

#include "../../src/self-registering-factory.h"
#include "shape.h"
#include <cmath>

class Rectangle: public SelfRegisteringClass<Rectangle,Shape> {
    float width,height;
public:
    Rectangle(float width = 1.0f, float height = 1.0f) : width(width), height(height) {}
    float area() const override { return width*height; }

    static std::string type_name() { return "rectangle"; }
};

