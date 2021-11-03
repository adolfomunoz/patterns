#pragma once

#include "shape.h"
#include "../../src/reflect-inheritance.h"
#include <cmath>

class Point: public pattern::ReflectableInheritance<Shape> {
public:
    float area() const override { return 0.0f; }
    static const char* type_name() { return "point"; }
    
};

