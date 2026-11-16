#pragma once

#include "../../patterns.h"
#include "shape.h"
#include <cmath>

class Circle : public pattern::SelfRegisteringClass<Circle,Shape>, public Shape {
    float radius;
public:
    //It is important that it has a default contructor, for the factory to be able to create it. 
    Circle(float radius = 1.0f) : radius(radius) {}
    float area() const override { return 3.14159276f*radius*radius; }

    static const char* type_name() { return "circle"; }
};

