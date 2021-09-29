#pragma once
#include "type-traits.h"
#include "../ext/rapidxml/rapidxml.hpp"
#include <sstream>

namespace pattern {
    
namespace {
    // SFINAE test
    template <typename T>
    class has_xml
    {
    private:
        typedef char YesType[1];
        typedef char NoType[2];

        template <typename C> static YesType& test( decltype(&C::xml) ) ;
        template <typename C> static NoType& test(...);


    public:
        static constexpr bool value = sizeof(test<T>(0)) == sizeof(YesType);
    };

}

template<typename T>
std::string xml(const T& t, const std::string& name = "", const std::string& prefix = "",
    std::enable_if_t<has_xml<T>::value,int> sfinae=0) {
    
    return t.xml(name,prefix);
}  

template<typename T>
std::string xml(const T& t, const std::string& name = "", const std::string& prefix = "", 
    std::enable_if_t<!has_xml<T>::value,int> sfinae=0) {
    std::stringstream sstr;
    sstr<<prefix<<"<"<<type_traits<T>::name()<<" ";
    if (name != "") sstr<<"name=\""<<name<<"\" ";
    sstr<<"value=\""<<t<<"\" />\n";
    return sstr.str();
}

template<typename T>
struct XML {
    static void load(T& t, rapidxml::xml_node<>* node) {
        rapidxml::xml_node<>* found = node->first_node(type_traits<T>::name());
        if (found) {
            rapidxml::xml_attribute<>* value = found->first_attribute("value");
            if (value) {
                std::string str(value->value(), value->value_size()); 
                std::istringstream ss(str);
                ss>>t;
            }
        }
    } 

    static void load(T& t, const std::string& xml) {
        std::string copy(xml);
        rapidxml::xml_document<> doc;
        doc.parse<0>(&copy[0]);
        load(t,&doc);
    }
};

template<typename T>
void load_xml(T& t, const std::string& xml) { XML<T>::load(t,xml); }


}
