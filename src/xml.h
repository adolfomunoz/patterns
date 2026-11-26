#pragma once
#include "io.h"
#include "reflect.h"
#include "rapidxml.h"
#include <unordered_map>



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
            std::stringstream whole;
            //This is for the CDATA and the subnodes, if any
            if (node->first_node()) { 
                for (rapidxml::xml_node<> *child = node->first_node(); child; child = child->next_sibling()) {
                    rapidxml::print(std::ostream_iterator<char>(whole), *child, rapidxml::print_no_indenting);
                }
            } else {
                whole<<std::string_view(node->value(),node->value_size());
            }  
            IO<T>::from_string(t,whole.str());
        }
        #ifdef PATTERN_LOG_XML
        else {
            std::cerr<<"[ WARN ] XML cannot load element of type "<<type_traits<T>::name();
            if (!att_name.empty()) std::cerr<<" with name "<<att_name;
            std::cerr<<std::endl;
        }
        #endif
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
        t.init();           
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

/**
 * Contains a default value
 */
struct Default : public XMLAble {
    static std::string get_inner_xml(const rapidxml::xml_node<>* node) {
    if (!node) return {};

    std::string inner_xml;
    for (const rapidxml::xml_node<>* child = node->first_node();
         child != nullptr;
         child = child->next_sibling()) {
        rapidxml::print(std::back_inserter(inner_xml), *child, rapidxml::print_no_indenting);
    }
    return inner_xml;
}
public:
    std::string name, value;
    Default(const std::string& name = "", const std::string& value = "") : name(name), value(value) {}

    void init() {}
    static const char* type_name() { return "default"; }
    void load(rapidxml::xml_node<>* node, const std::string& att_name = "") override {
        auto found = find_xml_node(node,"default",att_name);
        if (found) {
            rapidxml::xml_attribute<>* name_att = found->first_attribute("name");
            if (name_att) name = std::string(name_att->value(),name_att->value_size());
            rapidxml::xml_attribute<>* value_att = found->first_attribute("value");
            if (value_att) value = std::string(value_att->value(),value_att->value_size());
            else value = get_inner_xml(found);
        } 
    }
    virtual bool generates(xml_flag_type flags = 0) const override {
        return !name.empty();
    }
    std::string get_tag(const std::string&  name = "", xml_flag_type flags = 0) const override {
        return "default";
    }
    std::string get_attributes(const std::string&  name = "", xml_flag_type flags = 0) const override {
       if (!(flags & xml_value_as_content)) return " name=\""+this->name+"\" value=\""+value+"\" ";
       else return " name=\""+this->name+"\" ";
    }
    std::string get_content(const std::string&  name = "", const std::string& prefix = "", xml_flag_type flags = 0) const override {
       if (flags & xml_value_as_content) return prefix+value;
       else return "";
    }
}; 


inline void replace_string(std::string& subject, const std::string& search,
                        const std::string& replace) {
    size_t pos = 0;
    while ((pos = subject.find(search, pos)) != std::string::npos) {
            subject.replace(pos, search.length(), replace);
            pos += replace.length();
    }
}

inline size_t xml_tag_end(const std::string& xml, size_t begin) {
    char quote = '\0';
    for (size_t pos = begin; pos < xml.size(); ++pos) {
        if (quote) {
            if (xml[pos] == quote) quote = '\0';
        } else if (xml[pos] == '\'' || xml[pos] == '"') {
            quote = xml[pos];
        } else if (xml[pos] == '>') {
            return pos;
        }
    }
    return std::string::npos;
}

inline std::tuple<std::string, std::string> split_xml_nodes(const std::string& xml, std::string_view tag) {
    std::string xml_without_tag = xml;
    std::string xml_with_tag = "";
    
    const std::string opening = "<" + std::string(tag);
    const std::string closing = "</" + std::string(tag);
    auto is_boundary = [](char c) {
        return c == '>' || c == '/' || std::isspace(static_cast<unsigned char>(c));
    };

    size_t search_from = 0;
    while ((search_from = xml_without_tag.find(opening, search_from)) != std::string::npos) {
        const size_t opening_name_end = search_from + opening.size();
        if (opening_name_end >= xml.size() || !is_boundary(xml_without_tag[opening_name_end])) {
            ++search_from;
            continue;
        }

        const size_t opening_end = xml_tag_end(xml, opening_name_end);
        if (opening_end == std::string::npos) break;

        size_t slash = opening_end;
        while (slash > opening_name_end && std::isspace(static_cast<unsigned char>(xml_without_tag[slash - 1]))) --slash;
        if (slash > opening_name_end && xml_without_tag[slash - 1] == '/') {
            xml_with_tag += xml_without_tag.substr(search_from, opening_end - search_from + 1);
            xml_without_tag.erase(search_from, opening_end - search_from + 1);
            continue;
        }

        size_t closing_begin = xml.find(closing, opening_end + 1);
        while (closing_begin != std::string::npos) {
            const size_t closing_name_end = closing_begin + closing.size();
            if (closing_name_end < xml_without_tag.size() && is_boundary(xml_without_tag[closing_name_end])) break;
            closing_begin = xml_without_tag.find(closing, closing_name_end);
        }
        if (closing_begin == std::string::npos) {
            search_from = opening_end + 1;
            continue;
        }

        const size_t closing_end = xml_tag_end(xml_without_tag, closing_begin + closing.size());
        if (closing_end == std::string::npos) break;
        xml_with_tag += xml_without_tag.substr(search_from, closing_end - search_from + 1);
        xml_without_tag.erase(search_from, closing_end - search_from + 1);
    }
    return {xml_without_tag, xml_with_tag};
}

inline std::string replace_variables_in_xml(const std::string& xml, const std::list<Default>& variables = {}, std::string path = "") {
    auto [copy, copy_defaults] = split_xml_nodes(xml, "default");
    rapidxml::xml_document<> doc_defaults;
    std::list<Default> defaults;
    doc_defaults.parse<0>(&copy_defaults[0]);
    XML<std::list<Default>>::load(defaults,&doc_defaults);
    std::unordered_map<std::string,std::string> variable_map;
    for (const auto& d : defaults)  {
        variable_map[d.name] = d.value;
    }
    //The variables replace the default values obtained from the xml file
    for (const auto& d : variables) {
        variable_map[d.name] = d.value;
    }

    for (const auto& [name,value] : variable_map) {
        std::string replacement = value;
        if ((value.size() > 4) && (value.substr(value.length()-4,4) == ".xml")) {
            // If the value is a path to an XML file, we load it
            std::ifstream in(value);
            if (!in.is_open()) {
                // Try to open the file relative to the provided path
                std::string full_path = path + value;
                in.open(full_path);
            }
            if (in.is_open()) {
                std::ostringstream xml_sstr;
                xml_sstr << in.rdbuf();
                in.close();
                // Recursively replace variables in the loaded XML
                std::list<Default> nested_variables = variables; // Pass the same variables for nested replacement
                nested_variables.remove_if([&name](const Default& d) { return d.name == name; }); // Avoid self-replacement because it creates an infinite recursion
                std::string xml_value = xml_sstr.str();
                replace_variables_in_xml(xml_value,nested_variables,path);
                replacement = xml_value;
            } else {
                std::cerr << "Could not open XML file for variable replacement: " << value << std::endl;
            }
        }
        //First we replace full nodes with the variable names (we need to preserve
        // the inner xml of the file even before the replacement)
        replace_string(copy,std::string("<$")+name+"/>",replacement);
        //Once we have replaced full xml nodes, we replace the variables.
        replace_string(copy,std::string("$")+name,replacement);
    }

    return copy;
}

template<typename T>
void load_xml(T& t, const std::string& xml, std::list<Default> variables = {}, std::string path = "") { 
    std::string prepared_xml = replace_variables_in_xml(xml,variables,path);
    rapidxml::xml_document<> doc;
    doc.parse<0>(&prepared_xml[0]);
    XML<T>::load(t,&doc);    
    if constexpr (has_init_v<T>) t.init();
}

template<typename T>
bool load_xml_file(T& t, const std::string& xmlfile, std::list<Default> variables = {}) { 
    //Too many copies here, we will solve it later if needed.
    std::ifstream in(xmlfile);
    if (in.is_open()) {
        std::string xmlpath = xmlfile.substr(0,xmlfile.find_last_of("/\\")+1);
        std::ostringstream sstr;
        sstr << in.rdbuf();
        load_xml(t,sstr.str(),variables,xmlpath);
        return true;
    } else return false;
}

template<typename T>
T make_from_xml(const std::string& xml, std::list<Default> variables = {}) {
    T t; //Must be default constructible
    load_xml(t,xml,variables);
    return t;
}



}
