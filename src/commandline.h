#pragma once
#include "xml.h"
#include <sstream>
#include <vector>
#include <list>
#include <regex>
#include <optional>

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
struct CommandLine<std::optional<T>> {
    static void load(std::optional<T>& t, int argc, char** argv, const std::string& name = "") {
        std::string searchfor = name;
        if (searchfor.empty()) searchfor=type_traits<T>::name();
        for (int i = 1; i<argc; ++i) {
            auto tokens = tokenize(std::string(argv[i]),std::regex("="));
            if (tokens[0] == (std::string("--")+searchfor)) {
                std::stringstream s(tokens[1]);
                if constexpr (has_ostream_operator_v<T>) { //We need to account for lists, maybe
                    T data; s>>data; t=data;
                }
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



template<typename T>
struct CommandLine<T, std::enable_if_t<is_collection_v<T>>> { 
    static void load(T& t, int argc, char** argv, const std::string& name = "") {
        //We add only if something is going to be found, and only add one element
        bool do_add = false;
        std::string searchfor = name;
        if (searchfor.empty()) searchfor=type_traits<T>::name(); 
        for (int i = 1; i<argc; ++i) {
            if (std::string(argv[i]).rfind(std::string("--")+searchfor,0) == 0)
                do_add = true;
        }
        if (do_add) {
            typename T::value_type elem; //Just loads the first element of the collection for now
            CommandLine<typename T::value_type>::load(elem,argc,argv,name);
            if constexpr (is_pimpl_v<typename T::value_type>) {
                if (elem.impl()) t.push_back(elem); 
            } else { 
                t.push_back(elem);
            }
        }
    }    
};

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
                for (int j = 1; j<argc; ++j) {
                    auto tokens = tokenize(std::string(argv[j]),std::regex("="));
                    replace_string(xmlstring,std::string("$")+tokens[0].substr(2),tokens[1]);
                }
                load_xml(t,xmlstring);
            }
        }
    }
    
    CommandLine<T>::load(t,argc,argv,name);

    if constexpr (has_init_v<T>) t.init();
}

template<typename T>
T make_from_commandline(int argc, char** argv, const std::string& name = "") {
    T t; //Must be default constructible
    load_commandline(t,argc,argv,name);
    return t;
}


}
