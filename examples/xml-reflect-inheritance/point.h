#pragma once

#include "shape.h"
#include "../../src/reflect.h"
#include <cmath>

class Point: public pattern::Reflectable<Point,Shape> {
public:
    float area() const override { return 0.0f; }
    static const char* type_name() { return "point"; }
    
};

