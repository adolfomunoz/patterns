#include "rectangle.h"
#include "../../src/xml.h"
#include <iostream>

int main(int argc, char** argv) {
    Rectangle rect(2.0,2.0);
    std::cout<<"Area = "<<rect.area()<<std::endl;
    std::cout<<xml(rect)<<std::endl;
}

