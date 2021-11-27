#include "../../patterns.h"
#include "rectangle.h"
#include "circle.h"


int main(int argc, char** argv) {
    int integer = pattern::make_from_commandline<int>(argc,argv);
    std::cout<<"integer = "<<integer<<std::endl;
    float distance = pattern::make_from_commandline<float>(argc,argv,"distance");
    std::cout<<"distance = "<<distance<<std::endl;
    auto rect = pattern::make_from_commandline<Rectangle>(argc,argv);
    std::cout<<"rectangle of "<<rect.width()<<"x"<<rect.height()<<std::endl;
    auto shape = pattern::make_from_commandline<Shape>(argc,argv);
    std::cout<<pattern::xml(shape)<<std::endl;
}

