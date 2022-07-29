#pragma once

#include "shape.h"
#include <cmath>

template<std::size_t N>
class Regular: public pattern::Reflectable<Regular<N>,ShapeBase> {
protected:
    float side;
public:
    Regular(float side = 1.0f) : side(side) {}
    float area() const override { 
        return N*side/(2.0f*std::tan(float(M_PI)/float(N)));
    }
    auto reflect_names() const { return std::tuple("side"); }
    auto reflect() { return std::tie(side); } 
};

class Square : public pattern::Reflectable<Square,Regular<4>> {
public:
    using pattern::Reflectable<Square,Regular<4>>::Reflectable;
    static const char* type_name() { return "square"; }
};

class Pentagon : public pattern::Reflectable<Pentagon,Regular<5>> {
public:
    using pattern::Reflectable<Pentagon,Regular<5>>::Reflectable;
    static const char* type_name() { return "pentagon"; }
};



