#pragma once

#include "shape.h"
#include <cmath>
#include <iostream>

class Rectangle: public pattern::Visitable<Rectangle,ShapeBase> {
    float width_,height_;
public:
    Rectangle(float width = 1.0f, float height = 1.0f) : width_(width), height_(height) {}
    float width() const noexcept { return width_; }
    float height() const noexcept { return height_; }
    float area() const override { return width()*height(); }
};

std::ostream& operator<<(std::ostream& os, const Rectangle& rect) {
    os<<"Rectange = "<<rect.width()<<"x"<<rect.height()<<std::endl;
    return os;
}


