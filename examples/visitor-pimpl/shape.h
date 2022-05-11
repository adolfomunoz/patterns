#pragma once

#include "../../patterns.h"

class ShapeBase : public pattern::VisitableBase {
public:
    virtual float area() const = 0;
};

class Shape : public pattern::Pimpl<ShapeBase> {
public:
    using pattern::Pimpl<ShapeBase>::Pimpl;
    using pattern::Pimpl<ShapeBase>::operator=;

    float area() const override { return this->impl()->area(); }
};


