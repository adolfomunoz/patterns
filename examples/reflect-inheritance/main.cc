#include "rectangle.h"
#include "../../src/type-traits.h"
#include <iostream>

int main(int argc, char** argv) {
    Rectangle rect(2.0,2.0);
    std::cout<<"Area = "<<rect.area()<<" | Perimeter = "<<rect.perimeter()<<std::endl;
    
    // vv Lists all the base classes known at compile type from ReflectableInheritance.
    rect.for_all_base_classes([] (const auto& a) { 
        std::cout<<pattern::type_traits<decltype(a)>::name()<<'\n'; });
}

