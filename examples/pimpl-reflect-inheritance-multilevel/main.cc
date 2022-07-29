#include "circle.h"
#include "rectangle.h"
#include "regular.h"
#include <iostream>
#include <list>

int main(int argc, char** argv) {
    {
        Shape shape = Circle(2.0);
        std::cout<<pattern::xml(shape)<<std::endl;
    }
    
    {
        std::list<Shape> shapes;
        shapes.push_back(Circle(2.0));
        shapes.push_back(Rectangle(1.0,2.0));
        shapes.push_back(Square(3.0));
        shapes.push_back(Pentagon(1.0));
        std::cout<<pattern::xml(shapes)<<std::endl;
    }
    
    {
        Shape shape;
        pattern::load_xml(shape,
            "<shape type=\"rectangle\">\
                <float name=\"height\" value=\"1.0\"/>\
                <float name=\"width\" value=\"2.0\"/>\
             </shape>");
        std::cout<<pattern::xml(shape)<<std::endl;
    }

}

