#pragma once

#include "shape.h"
#include <cmath>

template<std::size_t N>
class Regular: public pattern::Reflectable<Regular<N>,ShapeBase> {
protected:
    float side;
public:
    Regular(float width = 1.0f, float height = 1.0f) : width(width), height(height) {}
    float area() const override { return width*height; }
    static const char* type_name() { return "rectangle"; }
    auto reflect_names() const { return std::tuple("width","height"); }
    auto reflect() { return std::tie(width,height); } 
};


