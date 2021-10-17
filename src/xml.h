#pragma once
#include "type-traits.h"
#include "reflect.h"
#include "../ext/rapidxml/rapidxml.hpp"
#include <sstream>
#include <vector>
#include <list>

namespace pattern {
    
/*
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
*/

template<typename T>
struct XMLSearch {
    static rapidxml::xml_node<>* find(rapidxml::xml_node<>* node, const std::string& att_name = "") {
        if (!node) return nullptr;
        rapidxml::xml_node<>* found = nullptr; 
        for (found = node->first_node(type_traits<T>::name()); 
             found; 
             found = found->next_sibling(type_traits<T>::name())) {
        
            if (att_name.empty()) return found;
            else {
                rapidxml::xml_attribute<>* name = found->first_attribute("name");
                if ((name) && (att_name == std::string(name->value(),name->value_size())))
                    return found;
            }
        }
        return nullptr;
    }
};

template<typename T, typename Enable = void>
struct XML {
    static void load(T& t, rapidxml::xml_node<>* node, const std::string& att_name = "") {
        rapidxml::xml_node<>* found = XMLSearch<T>::find(node,att_name);
        if (found) {
            rapidxml::xml_attribute<>* value = found->first_attribute("value");
            if (value) {
                std::string str(value->value(), value->value_size()); 
                std::istringstream ss(str);
                ss>>t;
            }
        } else if (!att_name.empty()) {
            rapidxml::xml_attribute<>* att = node->first_attribute(att_name.c_str());
            if (att) {
                std::string str(att->value(), att->value_size()); 
                std::istringstream ss(str);
                ss>>t;
            }
        }
    } 
    
    static std::string get(const T& t, const std::string& name = "", const std::string& prefix = "") {
        std::stringstream sstr;
        sstr<<prefix<<"<"<<type_traits<T>::name()<<" ";
        if (name != "") sstr<<"name=\""<<name<<"\" ";
        sstr<<"value=\""<<t<<"\" />\n";
        return sstr.str();
    }
};

template<typename T>
struct XML<T, std::enable_if_t<is_reflectable_v<T>>> {
    static void load(Reflectable<T>& t, rapidxml::xml_node<>* node, const std::string& att_name = "") {
        rapidxml::xml_node<>* found = XMLSearch<T>::find(node,att_name);
        if (found) {
            t.for_each_attribute([&found] (const std::string& name, auto& value) {
                XML<std::decay_t<decltype(value)>>::load(value,found,name);
            });                      
        }
    }

    static std::string get(const Reflectable<T>& t, const std::string& name = "", const std::string& prefix = "") {
        std::stringstream sstr;
        sstr<<prefix<<"<"<<type_traits<T>::name();
        if (name != "") sstr<<" name=\""<<name<<"\" ";
        sstr<<">\n";
       
        t.for_each_attribute([&sstr,&prefix] (const std::string& name, const auto& value) {
            sstr<<XML<std::decay_t<decltype(value)>>::get(value,name,prefix+"   ");
        });
        sstr<<prefix<<"</"<<type_traits<T>::name()<<">\n";
        return sstr.str();
    }    
};

namespace {
    template<typename T>
    struct is_collection_impl {
        static constexpr bool value = false;
    };
    
    template<typename T>
    struct is_collection_impl<std::list<T>> {
        static constexpr bool value = true;
    };
    
    template<typename T>
    struct is_collection_impl<std::vector<T>> {
        static constexpr bool value = true;
    };
    
    template<typename T>
    struct is_collection {
        static constexpr bool value = is_collection_impl<std::decay_t<T>>::value;
    };
    
    template<typename T>
    inline constexpr bool is_collection_v = is_collection<T>::value;
}

template<typename T>
struct XML<T, std::enable_if_t<is_collection_v<T>>> {
    static std::string get(const T& t, const std::string& name = "", const std::string& prefix = "") {
        std::stringstream sstr;
        for (const auto& item : t) {
            sstr<<XML<std::decay_t<decltype(item)>>::get(item,name,prefix);
        }
        return sstr.str();
    } 
    
    static void load(T& t, rapidxml::xml_node<>* node, const std::string& att_name = "") {
        if (node) {
            t.clear();
            rapidxml::xml_node<>* found = nullptr; 
            for (found = node->first_node(type_traits<typename T::value_type>::name()); 
                 found; 
                 found = found->next_sibling(type_traits<typename T::value_type>::name())) {
                
                rapidxml::xml_attribute<>* name = found->first_attribute("name");
                if ((att_name.empty()) || 
                    ((name) && (att_name == std::string(name->value(),name->value_size())))) {
                    
                    typename T::value_type value;
                    rapidxml::xml_document<> tmpdoc;
                    tmpdoc.append_node(tmpdoc.clone_node(found));
                    XML<typename T::value_type>::load(value,&tmpdoc,att_name);
                    t.push_back(value);
                }
            }
        }
    }    
};

template<typename T>
void load_xml(T& t, const std::string& xml) { 
    std::string copy(xml);
    rapidxml::xml_document<> doc;
    doc.parse<0>(&copy[0]);
    XML<T>::load(t,&doc);    
}

template<typename T>
T make_from_xml(const std::string& xml) {
    T t; //Must be default constructible
    load_xml(t,xml);
    return t;
}

template<typename T>
std::string xml(const T& t) {
    return XML<T>::get(t);
}


}
