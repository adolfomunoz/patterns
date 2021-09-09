#pragma once

#include "../../src/pimpl.h"

class ShapeBase {
public:
    virtual float area() const = 0;
};

class Shape : public pattern::Pimpl<ShapeBase> {
public:
    using pattern::Pimpl<ShapeBase>::Pimpl;
    
    float area() const override { return impl()->area(); }
};


