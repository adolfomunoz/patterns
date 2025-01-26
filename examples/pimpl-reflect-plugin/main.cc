//This is to show the process of registering
#define PATTERN_SHOW_SELF_REGISTERING

#include "rectangle.h"
#include <iostream>
#include <list>

int main(int argc, char** argv) {
    {
        std::list<Shape> shapes;
        shapes.push_back(Rectangle(1.0,2.0));
        //We cannot do this with Circle because it is compiled separately
        std::cout<<pattern::xml(shapes)<<std::endl;
    }

    { 
        Shape shape("circle");
        std::cout<<"Circle area = "<<shape.area()<<std::endl;
        std::cout<<pattern::xml(shape)<<std::endl;
    }
    
    {
        Shape shape;
        pattern::load_xml(shape,
            "<shape type=\"circle\">\
                <float name=\"radius\" value=\"3.0\"/>\
             </shape>");
        std::cout<<pattern::xml(shape)<<std::endl;
    }

}

