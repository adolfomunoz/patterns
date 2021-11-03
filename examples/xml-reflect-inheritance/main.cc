#include "rectangle.h"
#include "point.h"
#include "../../src/xml.h"
#include <iostream>

int main(int argc, char** argv) {
    Rectangle rect(2.0,2.0);
    std::cout<<"Rectangle, with area = "<<rect.area()<<std::endl;
    std::cout<<xml(rect)<<std::endl;
    Point point;
    std::cout<<"Point, with area = "<<point.area()<<std::endl;
    std::cout<<xml(point)<<std::endl;

}

