#pragma once
#include "../../src/reflect.h"

class Shape : public pattern::Reflectable<Shape> {
    float x, y;
public:
    Shape(): x(0), y(0) {}
    auto reflect_names() const { return std::tuple("x","y"); }
    auto reflect() { return std::tie(x,y); }
    virtual float area() const = 0;
    static const char* type_name() { return "shape"; }

};


