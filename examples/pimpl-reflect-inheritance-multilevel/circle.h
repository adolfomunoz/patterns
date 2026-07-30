#pragma once

#include "shape.h"
#include <cmath>


class Circle : public pattern::Reflectable<Circle,ShapeBase> {
protected:
    float radius;
public:
    Circle(float radius = 1.0f) : radius(radius) {}
    float area() const override { return 3.14159276f*radius*radius; }
    static const char* type_name() { return "circle"; }
    auto reflect_names() const { return std::tuple("radius"); }
    auto reflect() { return std::tie(radius);  }
};

class Sector : public pattern::Reflectable<Sector,Circle> {
    float angle;
public:
    Sector(float radius = 1.0f, float angle = 1.0f) : angle(angle) { this->radius = radius;}
    float area() const override { return Circle::area()*angle/(2.0f*3.14159276f); }
    static const char* type_name() { return "sector"; }
    auto reflect_names() const { return std::tuple("angle"); }
    auto reflect() { return std::tie(angle);  }
};

