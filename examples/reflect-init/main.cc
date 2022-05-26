#include "rectangle.h"
#include "../../src/xml.h"

int main() {
    Rectangle rect(1.0,2.0);
    std::cout<<pattern::xml(rect)<<std::endl;
    std::cout<<"Area = "<<rect.area()<<std::endl<<std::endl;
    //This should update the area as well
    pattern::load_xml(rect,"<rectangle width=\"1.0\" height=\"3.0\"/>");
    std::cout<<pattern::xml(rect)<<std::endl;
    std::cout<<"Area = "<<rect.area()<<std::endl;
};
