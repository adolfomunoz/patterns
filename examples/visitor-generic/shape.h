#pragma once

#include "../../patterns.h"

class Shape : public pattern::VisitableBase {
public:
    virtual float area() const = 0;
};


