#include "rectangle.h"
#include "circle.h"
#include "../../src/type-traits.h"
#include <iostream>

class PrintVisitor : public pattern::ConstVisitor<Circle,Rectangle> {
    void visit(const Rectangle& rect) {
        std::cout<<"Rectange = "<<rect.width()<<"x"<<rect.height()<<std::endl;
    }
    void visit(const Circle& circ) {
        std::cout<<"Circle   = "<<circ.radius()<<std::endl;
    }
};

int main(int argc, char** argv) {
    Rectangle rect(2.0,2.0);

    Shape s = rect; //It stores a copy of the rectangle
    
    PrintVisitor pv;
    s.accept(pv);

    std::cout<<"Area = "<<s.apply([] (Rectangle& rect) { return rect.area(); }, [] (Circle& circ) { return circ.area(); })<<std::endl;

    std::cout<<"Radius = "<<s.apply([] (Circle& circ) { return circ.radius(); }, -1.0f)<<std::endl;
}

