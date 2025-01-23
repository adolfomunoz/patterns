#pragma once

#include <iostream>

class Shape {
public:
    virtual float area() const = 0;
    static const char* type_name() { return "shape"; }
    virtual ~Shape() { }
};
