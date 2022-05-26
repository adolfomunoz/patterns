#include "circle.h"
#include "rectangle.h"
#include <iostream>
#include <list>

int main(int argc, char** argv) {
   Shape shape;
   pattern::load_xml(shape,
            "<shape type=\"rectangle\">\
                <float name=\"height\" value=\"1.0\"/>\
                <float name=\"width\" value=\"2.0\"/>\
             </shape>");
   std::cout<<pattern::xml(shape)<<std::endl;
   std::cout<<"Area = "<<shape.area()<<std::endl<<std::endl;
   pattern::load_xml(shape,
            "<shape type=\"circle\">\
                <float name=\"radius\" value=\"3.0\"/>\
             </shape>");
   std::cout<<pattern::xml(shape)<<std::endl;
   std::cout<<"Area = "<<shape.area()<<std::endl<<std::endl;
}

