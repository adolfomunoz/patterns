#include "rectangle.h"
#include "transformed.h"
#include "../../src/xml.h"

int main() {
    Rectangle rect(1.0,2.0);
    std::cout<<rect<<std::endl;
    
//    std::cin>>rect;
//    std::cout<<rect<<std::endl;
    
    std::cout<<pattern::xml(rect)<<std::endl;
    
    Transformed g(3,-2,Rectangle(5,5));
    std::cout<<pattern::xml(g)<<std::endl;

    int i;
    pattern::load_xml(i,"<integer value=\"3\"/>");
    std::cout<<i<<"\n";
    
    pattern::load_xml(rect,
        "<rectangle>\
            <float name=\"height\" value=\"1.0\"/>\
            <float name=\"width\" value=\"3.0\"/>\
         </rectangle>");
    std::cout<<pattern::xml(rect)<<std::endl;
};
