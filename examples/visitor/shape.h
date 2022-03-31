#pragma once

#include "../../patterns.h"

class Shape : public VisitableBase {
public:
    virtual float area() const = 0;
};


