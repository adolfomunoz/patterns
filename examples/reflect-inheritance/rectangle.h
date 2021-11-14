#pragma once

#include "area.h"
#include "perimeter.h"
#include "../../src/reflect.h"
#include <cmath>

class Rectangle: public pattern::Reflectable<Rectangle,Area,Perimeter> {
    float width,height;
public:
    Rectangle(float width = 1.0f, float height = 1.0f) : width(width), height(height) {}
    float area() const override { return width*height; }
    float perimeter() const override { return width*height; }
};

