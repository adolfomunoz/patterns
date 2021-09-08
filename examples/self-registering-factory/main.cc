#include "circle.h"
#include "rectangle.h"
#include <iostream>

int main(int argc, char** argv) {
    if (argc>1) {
        if (auto shape = pattern::SelfRegisteringFactory<Shape>::make_unique(argv[1]))
            std::cout<<shape->area()<<std::endl;
        else
            std::cout<<pattern::SelfRegisteringFactory<Shape>::registered()<<std::endl;
    } else 
        std::cout<<pattern::SelfRegisteringFactory<Shape>::registered()<<std::endl;  
}
