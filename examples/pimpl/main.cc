#include "circle.h"
#include "rectangle.h"
#include <iostream>
#include <list>

//Note that this function works for a list of any type that has an area() method. This includes Circle, Rectangle,
//and Shape (which is a Pimpl of ShapeBase, which has an area() method). Without the Pimpl pattern, this
//function would not work for the case of a base class because in order to have inheritance S would have to be a pointer type.
template<typename S>
float total_area(const std::list<S>& shapes) {
    float total = 0.0f;
    for (const auto& s : shapes) total += s.area();
    return total;
}

int main(int argc, char** argv) {
    Shape shape = Circle(2.0);
    std::cout<<shape.area()<<" ";
    shape = Rectangle(1.0,2.0);
    std::cout<<shape.area()<<std::endl;
    
    if (shape.instance_of<Circle>()) {
        std::cout<<"Shape is a Circle of radius" <<
            shape.cast_static<Circle>().radius() <<std::endl;
    }
    else if (shape.instance_of<Rectangle>()) {
        std::cout<<"Shape is a Rectangle of dimensions "<< 
            shape.cast_static<Rectangle>().width() << "x" <<
            shape.cast_static<Rectangle>().height() <<std::endl;
    }
    else {
        std::cout<<"Shape is neither a Circle nor a Rectangle"<<std::endl;
    }

    Shape shape_null;
    if (shape_null.is_null()) std::cout<<"Shape is null"<<std::endl;
    else std::cout<<"Shape is not null"<<std::endl;

    std::list<Shape> shapes;
    shapes.push_back(Circle(2.0));
    shapes.push_back(Rectangle(1.0,2.0)); 
    std::cout<<"Total area: "<<total_area(shapes)<<std::endl;
}

