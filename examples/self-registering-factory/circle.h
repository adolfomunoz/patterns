#pragma once

#include "../../src/self-registering-factory.h"
#include "shape.h"
#include <cmath>

class Circle : public pattern::SelfRegisteringClass<Circle,Shape> {
    float radius;
public:
    Circle(float radius = 1.0f) : radius(radius) {}
    float area() const override { return M_PI*radius*radius; }

    static std::string type_name() { return "circle"; }
};
