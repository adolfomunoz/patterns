#include "circle.h"
#include "rectangle.h"
#include <iostream>

int main(int argc, char** argv) {
    if (argc>1) std::cout<<pattern::SelfRegisteringFactory<Shape>::create(argv[1])->area()<<std::endl;
    
}
