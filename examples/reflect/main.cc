#include "rectangle.h"
#include "transformed.h"
#include "../../src/xml.h"

int main() {
    Rectangle rect(1.0,2.0);
//    vv This does not work anymore because it could happen that some
//          attribute was not "outputable" iself, which was a bug
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
    pattern::load_xml(rect,"<rectangle width=\"1.0\" height=\"3.0\"/>");
    std::cout<<pattern::xml(rect)<<std::endl;
    pattern::load_xml(g,
        "<transformed>\
            <rectangle>\
                <float name=\"height\" value=\"1.0\"/>\
            </rectangle>\
        </transformed>");
    std::cout<<pattern::xml(g)<<std::endl;
    
    /*** LISTS ***/
    std::list<float> l{1.0f,2.0f,3.0f};
    std::cout<<pattern::xml(l)<<std::endl;
    pattern::load_xml(l,
        "<float value=\"-1.0\" /><float value=\"-2.0\" /><float value=\"-3.0\" />");
    std::cout<<pattern::xml(l)<<std::endl;
};
