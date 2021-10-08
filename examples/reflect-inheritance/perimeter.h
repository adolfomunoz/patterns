#pragma once

class Perimeter {
public:
    virtual float perimeter() const = 0;
    static const char* type_name() { return "perimeter"; }
};

