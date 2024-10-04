#pragma once

#include "shape.h"
#include <cmath>
#include <ostream>

class Circle : public pattern::Visitable<Circle,ShapeBase> {
    float radius_;
public:
    Circle(float radius = 1.0f) : radius_(radius) {}
    float radius() const noexcept { return radius_; }
    float area() const override { return 3.14159276f*radius()*radius(); }
};

std::ostream& operator<<(std::ostream& os, const Circle& circle) {
    os<<"Circle   = "<<circle.radius()<<std::endl;
    return os;
}

