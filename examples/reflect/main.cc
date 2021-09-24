#include "rectangle.h"
#include "transformed.h"

int main() {
    Rectangle rect(1.0,2.0);
    std::cout<<rect<<std::endl;
    
//    std::cin>>rect;
//    std::cout<<rect<<std::endl;
    
    std::cout<<rect.xml()<<std::endl;
    
    Transformed g(3,-2,Rectangle(5,5));
    std::cout<<g.xml()<<std::endl;
};
