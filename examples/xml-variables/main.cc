#include "../../patterns.h"
#include "rectangle.h"
#include "circle.h"

std::string xml_shapes = R"(
<default name="finalshape">
    <shape type="circle" radius="3"/>
</default>
<shape type="circle" radius="1"/>
<shape type="circle" radius="2"/>
<$finalshape/>
)";

std::string replacement_1 = R"(
<shape type="rectangle" width="2" height="3"/>
)";

std::string replacement_2 = R"(
<shape type="rectangle" width="4" height="5"/>
)";

std::string replacement_3 = R"(../examples/xml-variables/other_shape.xml)";

int main(int argc, char** argv) {
    std::list<Shape> shapes;
    pattern::load_xml(shapes,xml_shapes);
    std::cout<<"Shapes loaded from xml without replacements:"<<std::endl;
    std::cout<<pattern::xml(shapes)<<std::endl;

    pattern::load_xml(shapes,xml_shapes,{{"finalshape",replacement_1}});
    std::cout<<"Shapes loaded from xml with replacement 1:"<<std::endl;
    std::cout<<pattern::xml(shapes)<<std::endl;

    pattern::load_xml(shapes,xml_shapes,{{"finalshape",replacement_2}});
    std::cout<<"Shapes loaded from xml with replacement 2:"<<std::endl;
    std::cout<<pattern::xml(shapes)<<std::endl;

    pattern::load_xml(shapes,xml_shapes,{{"finalshape",replacement_3}});
    std::cout<<"Shapes loaded from xml with replacement 3:"<<std::endl;
    std::cout<<pattern::xml(shapes)<<std::endl;
}

