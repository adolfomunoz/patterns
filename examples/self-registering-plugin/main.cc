//This is to show the process of registering
#define PATTERN_SHOW_SELF_REGISTERING

#include "rectangle.h"
#include <iostream>

int main(int argc, char** argv) {
    // vv This is self-registered inside the main executable
    std::unique_ptr<Shape> shape_rectangle = pattern::SelfRegisteringFactory<Shape>::make_unique("rectangle");
    // vv This is registered on an external plugin
    std::unique_ptr<Shape> shape_circle = pattern::SelfRegisteringFactory<Shape>::make_unique("circle","self-registering-plugin-circle");

    std::cout<<"Rectangle area       = "<<shape_rectangle->area()<<std::endl;
    std::cout<<"Circle (plugin) area = "<<shape_circle->area()<<std::endl;
}
