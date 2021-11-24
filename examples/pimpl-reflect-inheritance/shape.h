#pragma once

#include "../../patterns.h"

class ShapeBase : public pattern::SelfRegisteringReflectableBase {
public:
    static const char* type_name() { return "shape"; }
    virtual float area() const = 0;
};

class Shape : public pattern::Pimpl<ShapeBase> {
public:
    using pattern::Pimpl<ShapeBase>::Pimpl;
    
    float area() const override { return impl()->area(); }
};


