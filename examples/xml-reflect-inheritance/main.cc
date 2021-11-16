#include "rectangle.h"
#include "point.h"
#include "../../src/xml.h"
#include <iostream>

int main(int argc, char** argv) {
    Rectangle rect(2.0,2.0);
    std::cout<<"Rectangle, with area = "<<rect.area()<<std::endl;
    std::cout<<pattern::xml(rect)<<std::endl;
    pattern::load_xml(rect,
        "<rectangle>\
            <float name=\"height\" value=\"1.0\"/>\
            <float name=\"x\" value=\"3.0\"/>\
         </rectangle>");
    std::cout<<pattern::xml(rect)<<std::endl;
         
    Point point;
    std::cout<<"Point, with area = "<<point.area()<<std::endl;
    std::cout<<pattern::xml(point)<<std::endl;
    pattern::load_xml(point,
        "<point>\
            <float name=\"x\" value=\"3.0\"/>\
         </point>");
    std::cout<<pattern::xml(point)<<std::endl;
    
}

