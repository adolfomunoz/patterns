#pragma once
#include "xml.h"
#include <sstream>
#include <vector>
#include <list>
#include <regex>

namespace pattern {
    

namespace {
    // SFINAE test
    template <typename T>
    class has_load_commandline
    {
    private:
        typedef char YesType[1];
        typedef char NoType[2];

        template <typename C> static YesType& test( decltype(&C::load_commandline) ) ;
        template <typename C> static NoType& test(...);


    public:
        static constexpr bool value = sizeof(test<T>(0)) == sizeof(YesType);
    };
    
    template <typename T>
    inline constexpr bool has_load_commandline_v = has_load_commandline<T>::value;
    
    /**
     * @brief Tokenize the given vector 
       according to the regex
     * and remove the empty tokens.
     *
     * @param str
     * @param re
     * @return std::vector<std::string>
     */
    std::vector<std::string> tokenize(
                         const std::string str,
                              const std::regex re)
    {
        std::sregex_token_iterator it{ str.begin(), 
                                 str.end(), re, -1 };
        std::vector<std::string> tokenized{ it, {} };
      
        // Additional check to remove empty strings
        tokenized.erase(
            std::remove_if(tokenized.begin(), 
                                tokenized.end(),
                           [](std::string const& s) {
                               return s.size() == 0;
                           }),
            tokenized.end());
      
        return tokenized;
    }
    
    void replace_string(std::string& subject, const std::string& search,
                          const std::string& replace) {
        size_t pos = 0;
        while ((pos = subject.find(search, pos)) != std::string::npos) {
             subject.replace(pos, search.length(), replace);
             pos += replace.length();
        }
    }
}


template<typename T, typename Enable = void>
struct CommandLine {
    static void load(T& t, int argc, char** argv, const std::string& name = "") {
        std::string searchfor = name;
        if (searchfor.empty()) searchfor=type_traits<T>::name();
        for (int i = 1; i<argc; ++i) {
            auto tokens = tokenize(std::string(argv[i]),std::regex("="));
            if (tokens[0] == (std::string("--")+searchfor)) {
                std::stringstream s(tokens[1]); 
                s>>t;
            }
        }
    }
};


template<typename T>
struct CommandLine<T, std::enable_if_t<is_reflectable_v<T>>> {
    static void load(T& t, int argc, char** argv, const std::string& att_name = "") {
        t.for_each_attribute([&att_name,argc,argv] (const std::string& name, auto& value) {
            if (att_name.empty()) { //Load directly or by putting the type name in the command line
                CommandLine<std::decay_t<decltype(value)>>::load(value,argc,argv,name);
                CommandLine<std::decay_t<decltype(value)>>::load(value,argc,argv,std::string(type_traits<T>::name())+"-"+name);
            } else {
                CommandLine<std::decay_t<decltype(value)>>::load(value,argc,argv,att_name+"-"+name);                
            }
                
        });          
    }
};


/*
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
*/

template<typename T>
struct CommandLine<T, std::enable_if_t<has_load_commandline_v<T>>> {
    static void load(T& t, int argc, char** argv, const std::string& name = "") {
        t.load_commandline(argc,argv,name);
    }    
};


template<typename T>
void load_commandline(T& t, int argc, char** argv, const std::string& name = "") { 
    std::string xmlstring;
    for (int i = 1; i<argc; ++i) {
        std::string arg(argv[i]);
        if (arg.substr(arg.length()-4,4) == ".xml") {
            std::ifstream in(arg);
            if (in.is_open()) {
                std::ostringstream sstr;
                sstr << in.rdbuf();
                xmlstring = sstr.str();
            }
        }
    }
    
    if (xmlstring.empty()) {
        CommandLine<T>::load(t,argc,argv,name);
    } else {
        for (int i = 1; i<argc; ++i) {
            auto tokens = tokenize(std::string(argv[i]),std::regex("="));
            replace_string(xmlstring,std::string("$")+tokens[0].substr(2),tokens[1]);
        } 
        load_xml(t,xmlstring);
    }
}

template<typename T>
T make_from_commandline(int argc, char** argv, const std::string& name = "") {
    T t; //Must be default constructible
    load_commandline(t,argc,argv,name);
    return t;
}


}
