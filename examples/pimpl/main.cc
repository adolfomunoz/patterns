#include "circle.h"
#include "rectangle.h"
#include <iostream>
#include <list>

int main(int argc, char** argv) {
    Shape shape = Circle(2.0);
    
    std::list<Shape> shapes;
    shapes.push_back(Circle(2.0));
    shapes.push_back(Rectangle(1.0,2.0));
    
    for (const auto& s : shapes) std::cout<<s.area()<<" ";
    std::cout<<std::endl;
}

