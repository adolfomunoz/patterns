#pragma once

#include "../../patterns.h"
#include "shape.h"
#include <cmath>

class Rectangle: public pattern::SelfRegisteringClass<Rectangle,Shape>, public Shape {
    float width,height;
public:
    //It is important that it has a default contructor, for the factory to be able to create it. 
    Rectangle(float width = 1.0f, float height = 1.0f) : width(width), height(height) {}
    float area() const override { return width*height; }

    static const char* type_name() { return "rectangle"; }
};

