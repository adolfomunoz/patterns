#pragma once
#include "type-traits.h"
#include "reflect.h"
#include <rapidxml/rapidxml.hpp>
#include <sstream>
#include <fstream>


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
    template <typename T>
    class has_load_xml
    {
    private:
        typedef char YesType[1];
        typedef char NoType[2];

        template <typename C> static YesType& test( decltype(&C::load_xml) ) ;
        template <typename C> static NoType& test(...);


    public:
        static constexpr bool value = sizeof(test<T>(0)) == sizeof(YesType);
    };
    
    template<typename T>
    struct provides_xml {
        static constexpr bool value = (has_xml<T>::value) && (has_load_xml<T>::value);
    };
    
    template<typename T>
    inline constexpr bool provides_xml_v = provides_xml<T>::value;   

    namespace has_insertion_operator_impl {
        typedef char YesType[2];
        typedef char NoType;

        struct any_t {
            template<typename T> any_t( T const& );
        };

        inline NoType operator<<( std::ostream const&, any_t const& ) { return NoType(); };

        template<typename T>
        struct has_insertion_operator {
            static std::ostream &s;
            static T const &t;
            static YesType& test(std::ostream&) ;
            static NoType& test(NoType);
            static bool const value = sizeof(test(s << t) ) == sizeof(YesType);
        };
    }

    template<typename T>
    struct has_ostream_operator :
        has_insertion_operator_impl::has_insertion_operator<std::decay_t<T>> {
    };
    
    template<typename T>
    inline constexpr bool has_ostream_operator_v = has_ostream_operator<T>::value;   
}





using xml_flag_type = unsigned char;
constexpr xml_flag_type xml_reflect_attributes_from_stream = 1;


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
    
    static std::string get(const T& t, const std::string& name = "", const std::string& prefix = "", xml_flag_type flags = 0) {
        std::stringstream sstr;
        sstr<<prefix<<"<"<<type_traits<T>::name()<<" ";
        if (name != "") sstr<<"name=\""<<name<<"\" ";
        sstr<<"value=\""<<t<<"\" />\n";
        return sstr.str();
    }
};

template<typename T>
struct XML<T, std::enable_if_t<is_reflectable_v<T>>> {
    static void load(T& t, rapidxml::xml_node<>* node, const std::string& att_name = "") {
        rapidxml::xml_node<>* found = XMLSearch<T>::find(node,att_name);
        if (found) {
            t.for_each_attribute([&found] (const std::string& name, auto& value) {
                XML<std::decay_t<decltype(value)>>::load(value,found,name);
            });          
        }
    }

    static std::string get(const T& t, const std::string& name = "", const std::string& prefix = "", xml_flag_type flags = 0) {
        std::stringstream sstr;
        sstr<<prefix<<"<"<<type_traits<T>::name();
        if (name != "") sstr<<" name=\""<<name<<"\" ";
        if (flags & xml_reflect_attributes_from_stream) {
            t.for_each_attribute([&sstr,&prefix] (const std::string& name, const auto& value) {
                if constexpr (has_ostream_operator_v<decltype(value)>)
                    sstr<<" "<<name<<"=\""<<value<<"\"";
            });
        }
        sstr<<">\n";
       
        t.for_each_attribute([&sstr,&prefix,&flags] (const std::string& name, const auto& value) {
            if ((!(flags & xml_reflect_attributes_from_stream)) || (!has_ostream_operator_v<decltype(value)>))
                sstr<<XML<std::decay_t<decltype(value)>>::get(value,name,prefix+"   ",flags);
        });
        sstr<<prefix<<"</"<<type_traits<T>::name()<<">\n";
        return sstr.str();
    }    
};

template<typename T>
struct XML<T, std::enable_if_t<is_collection_v<T>>> {
    static std::string get(const T& t, const std::string& name = "", const std::string& prefix = "", xml_flag_type flags = 0) {
        std::stringstream sstr;
        for (const auto& item : t) {
            sstr<<XML<std::decay_t<decltype(item)>>::get(item,name,prefix,flags);
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
                    if constexpr (is_pimpl_v<typename T::value_type>) {
                        if (value.impl()) t.push_back(value); //Only pushes valid pointer pimpls
                    } else {
                        t.push_back(value);
                    }
                }
            }
        }
    }    
};

template<typename T>
struct XML<T, std::enable_if_t<provides_xml_v<T>>> {
    static std::string get(const T& t, const std::string& name = "", const std::string& prefix = "", xml_flag_type flags = 0) {
        return t.xml(name,prefix);
    } 
    
    static void load(T& t, rapidxml::xml_node<>* node, const std::string& att_name = "") {
        if (node) t.load_xml(node,att_name);
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
bool load_xml_file(T& t, const std::string& xmlfile) { 
    //Too many copies here, we will solve it later if needed.
    std::ifstream in(xmlfile);
    if (in.is_open()) {
        std::ostringstream sstr;
        sstr << in.rdbuf();
        load_xml(t,sstr.str());
        return true;
    } else return false;
}

template<typename T>
T make_from_xml(const std::string& xml) {
    T t; //Must be default constructible
    load_xml(t,xml);
    return t;
}

template<typename T>
std::string xml(const T& t, xml_flag_type flags = 0) {
    return XML<T>::get(t,"","",flags);
}


}
