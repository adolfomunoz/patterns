#pragma once
#include "type-traits.h"
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


}
