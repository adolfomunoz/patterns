#include "rectangle.h"
#include "circle.h"
#include "../../src/type-traits.h"
#include <iostream>

int main(int argc, char** argv) {
    Rectangle rect(2.0,2.0);

    Shape s = rect;
    
    s.apply_for<Circle,Rectangle>([] (const auto& shape) { std::cout<<shape; });
    std::cout<<"Area = "<<s.apply([] (Rectangle& rect) { return rect.area(); }, [] (Circle& circ) { return circ.area(); })<<std::endl;
    std::cout<<"Radius = "<<s.apply([] (Circle& circ) { return circ.radius(); }, -1.0f)<<std::endl;

}

