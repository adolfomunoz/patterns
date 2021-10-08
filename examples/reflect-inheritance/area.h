#pragma once

class Area {
public:
    virtual float area() const = 0;
    static const char* type_name() { return "area"; }

};


