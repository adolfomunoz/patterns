#pragma once
#include "io.h"
#include "reflect.h"
#include <rapidxml/rapidxml.hpp>



namespace pattern {

using xml_flag_type = unsigned char;
/** 
 * If activated, types that have operator<< will be output as attributes instead of children.
 * If activated, the first nameless attribute will be output as content of the node (if there is no
 * other content)
 */ 
constexpr xml_flag_type xml_reflect_attributes_from_stream = 1; 
/** 
 * If activated, polymorphic reflectable objetcs will have the tag of the derived class.
 * If not activated, the output tag is the one of the base class, and the derived class will
 * appear as attribute "type"
 */
constexpr xml_flag_type xml_tag_as_derived = 2;
/**
 * If activated, the values of types that have operator<< are output as node content
 * If not activated, the values are output as attribute "value"
 * This flag is ignored if "xml_reflect_attributes_from_stream" is activated 
 */
constexpr xml_flag_type xml_value_as_content = 4;

class XMLAble {
public:
    virtual void load(rapidxml::xml_node<>* node, const std::string& att_name = "") = 0;
    virtual bool generates(xml_flag_type flags = 0) const = 0;
    virtual std::string get_tag(const std::string&  name = "", xml_flag_type flags = 0) const = 0; 
    virtual std::string get_attributes(const std::string&  name = "", xml_flag_type flags = 0) const = 0;
    virtual std::string get_content(const std::string&  name = "", const std::string& prefix = "", xml_flag_type flags = 0) const = 0;
};


inline rapidxml::xml_node<>* find_xml_node(rapidxml::xml_node<>* node, const std::string& tag, std::string_view att_name = "") {
    if (!node) return nullptr;
    rapidxml::xml_node<>* found = nullptr; 
    for (found = node->first_node(tag.c_str()); found; found = found->next_sibling(tag.c_str())) {
        if (att_name.empty()) return found;
        else {
            rapidxml::xml_attribute<>* name = found->first_attribute("name");
            if ((name) && (att_name == std::string_view(name->value(),name->value_size())))
                return found;
        }
    }
    return nullptr;   
}



template<typename T, typename Enable = void>
struct XML {
    static constexpr bool available = false;
};

template<typename T>
struct XML<T,std::enable_if_t<IO<T>::available && !is_collection_v<T> && !is_reflectable_v<T>>> {
    static constexpr bool available = true;
    //Needed for reflectable IO<T>s
    static void load_content(T& t, rapidxml::xml_node<>* found) {}
    static void load(T& t, rapidxml::xml_node<>* node, const std::string& att_name = "") {
        auto found = find_xml_node(node,type_traits<T>::name(),att_name);
        if (found) {
            //Read from "value" attribute
            rapidxml::xml_attribute<>* value = found->first_attribute("value");
            if (value) IO<T>::from_string(t,std::string_view(value->value(),value->value_size()));
            else { //Read from node content
                IO<T>::from_string(t,std::string_view(found->value(),found->value_size()));
            }
        } else if (!att_name.empty()) { 
            //If there is no full node with this item, we try to find it as attribute of the "parent" node
            rapidxml::xml_attribute<>* att = node->first_attribute(att_name.c_str());
            if (att) IO<T>::from_string(t,std::string_view(att->value(),att->value_size()));
        } else if (node->value_size()>0) {
            //If it has no name and has not been loaded before we try to load it from content
            IO<T>::from_string(t,std::string_view(node->value(),node->value_size()));
        }
    }

    static bool generates(const T& t, xml_flag_type flags = 0) {
        return !IO<T>::to_string(t).empty();
    }

    static std::string get_tag(const T& t, std::string_view name = "", xml_flag_type flags = 0) {
        return type_traits<T>::name();
    }
    static std::string get_attributes(const T& t, std::string_view name = "", xml_flag_type flags = 0) {
        std::stringstream sstr;
        if (!name.empty()) sstr<<"name=\""<<name<<"\" ";
        if (!(flags & xml_value_as_content)) sstr<<"value=\""<<IO<T>::to_string(t)<<"\" ";
        return sstr.str();
    }
    static std::string get_content(const T& t, std::string_view name = "", std::string_view prefix = "", xml_flag_type flags = 0) {
        if (flags & xml_value_as_content) return IO<T>::to_string(t);
        else return "";
    }
};

template<typename T>
std::string xml(const T& t, const std::string& name = "", const std::string& prefix = "", xml_flag_type flags = 0) {
    if (XML<T>::generates(t,flags)) {
        std::stringstream sstr;
        std::string tag = XML<T>::get_tag(t,name,flags);
        std::string attributes = XML<T>::get_attributes(t,name,flags);
        std::string content = XML<T>::get_content(t,name,prefix,flags);
        if (tag.empty()) sstr<<content<<"\n";
        else {
            sstr<<prefix<<"<"<<tag;
            if (!attributes.empty()) sstr<<" "<<attributes;
            if (content.empty()) sstr<<"/>\n";
            else sstr<<">\n"<<content<<prefix<<"</"<<tag<<">\n";
        }
        return sstr.str();
    } else return "";
}

template<typename T>
std::string xml(const T& t, xml_flag_type flags) {
    return xml(t,"","",flags);
}   

template<typename T>
struct XML<T, std::enable_if_t<is_reflectable_v<T>>> {
    static constexpr bool available = true;
    static void load_content(T& t, rapidxml::xml_node<>* found) {
        if (found) {
            t.for_each_attribute([&found] (const std::string& name, auto& value) {
                XML<std::decay_t<decltype(value)>>::load(value,found,name);
            });  
        }
    }
    static void load(T& t, rapidxml::xml_node<>* node, std::string_view att_name = "") {
        auto found = find_xml_node(node,type_traits<T>::name(),att_name);
        load_content(t,found);
    }

    static bool generates(const T& t, xml_flag_type flags = 0) {
        return true;
    }
    static std::string get_tag(const T& t, std::string_view name = "", xml_flag_type flags = 0) {
        return type_traits<T>::name();
    }
    static std::string get_attributes(const T& t, std::string_view name = "", xml_flag_type flags = 0) {
        std::stringstream sstr;
        if (!name.empty()) sstr<<"name=\""<<name<<"\" ";
        if (flags & xml_reflect_attributes_from_stream) {
            t.for_each_attribute([&sstr] (const std::string& name, const auto& value) {
                if constexpr (IO<std::decay_t<decltype(value)>>::available) {
                    std::string ts = IO<std::decay_t<decltype(value)>>::to_string(value);
                    if ((!name.empty()) && (!ts.empty())) sstr<<name<<"=\""<<ts<<"\" ";
                }
            });
        }  
        return sstr.str();
    }

    static std::string get_content(const T& t, std::string_view name = "", const std::string& prefix = "", xml_flag_type flags = 0) {
        std::stringstream sstr;
        
        if (flags & xml_reflect_attributes_from_stream) {
            unsigned int unnamed_io_attributes = 0, xml_attributes = 0;
            t.for_each_attribute([&unnamed_io_attributes,&xml_attributes] (const std::string& name, const auto& value) {
                if (!IO<std::decay_t<decltype(value)>>::available) ++xml_attributes;
                if (name.empty() && (IO<std::decay_t<decltype(value)>>::available)) ++unnamed_io_attributes;
            });
            if ((xml_attributes == 0) && (unnamed_io_attributes ==1)) { 
                //Output the only attribute as content
                t.for_each_attribute([&sstr] (const std::string& name, const auto& value) {
                    if (name.empty() && (IO<std::decay_t<decltype(value)>>::available)) 
                        sstr<<IO<std::decay_t<decltype(value)>>::to_string(value);
                });   
            } else { 
                //We output general attributes and unnamed io attributes as xml 
                t.for_each_attribute([&sstr,&prefix,&flags] (const std::string& name, const auto& value) {
                    if (name.empty() || (!IO<std::decay_t<decltype(value)>>::available))
                        sstr<<prefix<<xml(value,name,prefix+"   ",flags);
                }); 
            }
        } else { //We output all attributes with full xml
            t.for_each_attribute([&sstr,&prefix,&flags] (const std::string& name, const auto& value) {
                sstr<<prefix<<xml(value,name,prefix+"    ",flags);
            });         
        }
        return sstr.str();
    }
};

template<typename T>
struct XML<T, std::enable_if_t<is_collection_v<T>>> {
    static constexpr bool available = true;
    static void load(T& t, rapidxml::xml_node<>* node, const std::string& att_name = "") {
        if (node) {
            t.clear();
            rapidxml::xml_node<>* found = nullptr; 
            for (found = node->first_node(); found; found = found->next_sibling()) {
                if (std::is_base_of_v<XMLAble,typename T::value_type> || 
                    (std::string_view(found->name(),found->name_size()) == type_traits<typename T::value_type>::name())) { 
                    
                    rapidxml::xml_attribute<>* name = found->first_attribute("name");
                    if ((att_name.empty()) || 
                        ((name) && (att_name == std::string_view(name->value(),name->value_size())))) {
                        
                        typename T::value_type value;
                        rapidxml::xml_document<> tmpdoc;
                        tmpdoc.append_node(tmpdoc.clone_node(found));
                        XML<typename T::value_type>::load(value,&tmpdoc,att_name);
                        //Only push valid values
                        if (XML<typename T::value_type>::generates(value)) t.push_back(value); //Only pushes valid pointer pimpls
                    }
                }
            } 
            if constexpr (IO<T>::available) { 
                auto found = find_xml_node(node,type_traits<T>::name(),att_name);
                if (found) {
                    //Read from "value" attribute
                    rapidxml::xml_attribute<>* value = found->first_attribute("value");
                    if (value) IO<T>::from_string(t,std::string_view(value->value(),value->value_size()));
                    else { //Read from node content
                        IO<T>::from_string(t,std::string_view(found->value(),found->value_size()));
                    }
                } else if (!att_name.empty()) { 
                    //If there is no full node with this item, we try to find it as attribute of the "parent" node
                    rapidxml::xml_attribute<>* att = node->first_attribute(att_name.c_str());
                    if (att) IO<T>::from_string(t,std::string_view(att->value(),att->value_size()));
                } else if (node->value_size()>0) {
                    // If it is not an attribute because it has no name, nor it is a subnode, 
                    // we try to load it as the node's content
                    IO<T>::from_string(t,std::string_view(node->value(),node->value_size()));
                }
            }
        }              
    }

    static bool generates(const T& t, xml_flag_type flags = 0) {
        return !t.empty();
    }

    static std::string get_tag(const T& t, std::string_view name = "", xml_flag_type flags = 0) {
        return "";
    }

    static std::string get_attributes(const T& t, std::string_view name = "", xml_flag_type flags = 0) {
        if constexpr (IO<T>::available) {
            std::stringstream sstr;
            if (flags & xml_reflect_attributes_from_stream) {
                if (!name.empty()) sstr<<"name=\""<<name<<"\" ";
                if (!(flags & xml_value_as_content)) sstr<<"value=\""<<IO<T>::to_string(t)<<"\" ";
            }
            return sstr.str();
        } else return "";
    }
    static std::string get_content(const T& t, const std::string& name = "", const std::string& prefix = "", xml_flag_type flags = 0) {
        if constexpr (IO<T>::available) {
            if ((flags & xml_reflect_attributes_from_stream) && (flags & xml_value_as_content)) return IO<T>::to_string(t);
        }
        std::stringstream sstr;
        for (const auto& item : t) {
            sstr<<xml(item,name,prefix,flags);
        }
        return sstr.str();
    }      
};

template<typename T>
struct XML<T, std::enable_if_t<std::is_base_of_v<XMLAble,T> && (!is_reflectable_v<T>) && (!IO<T>::available)>> {
    static constexpr bool available = true;
    static void load(T& t, rapidxml::xml_node<>* node, const std::string& att_name = "") {
        t.load(node,att_name);           
    }

    static bool generates(const T& t, xml_flag_type flags = 0) {
        return t.generates(flags);
    }

    static std::string get_tag(const T& t, const std::string&  name = "", xml_flag_type flags = 0) {
        return t.get_tag(name,flags);
    }

    static std::string get_attributes(const T& t, const std::string& name = "", xml_flag_type flags = 0) {
        return t.get_attributes(name,flags);
    }
    static std::string get_content(const T& t, const std::string& name = "", const std::string& prefix = "", xml_flag_type flags = 0) {
        return t.get_content(name,prefix,flags);
    }      
};

template<typename T>
void load_xml(T& t, const std::string& xml) { 
    std::string copy(xml);
    rapidxml::xml_document<> doc;
    doc.parse<0>(&copy[0]);
    XML<T>::load(t,&doc);    
    if constexpr (has_init_v<T>) t.init();
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



}
