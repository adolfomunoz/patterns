#pragma once

class Shape {
public:
    static const char* type_name() { return "shape"; }
    virtual ~Shape() = default; //The virtual destructor is obligatory for polymorphic classes but specially important for the base class of self-registering classes 
    virtual float area() const = 0;
};
