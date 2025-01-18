#pragma once
#include "type-traits.h"
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
    inline std::vector<std::string> tokenize(
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
}


template<typename T, typename Enable = void>
struct CommandLine {
    static_assert(has_ostream_operator_v<T>,"Type does not have ostream operator");
    static std::string help(const std::string& name = "", const std::string& prefix = "") {
        return prefix + "--" + (name.empty()?type_traits<T>::name():name)
            + "=<" + type_traits<T>::name() + ">";      
    } 

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

/** 
 * 
 * For some reason this gets completely messed up with collections...
template<typename T>
struct CommandLine<T, std::enable_if_t<IO<T>::available && (!is_collection_v<T>),int>> {
    static std::string help(const std::string& name = "", const std::string& prefix = "") {
        return prefix + "--" + (name.empty()?type_traits<T>::name():name)
            + "=<" + type_traits<T>::name() + ">";      
    } 
    
    static void load(T& t, int argc, char** argv, const std::string& att_name = "") {
        std::string searchfor = att_name;
        if (searchfor.empty()) searchfor=type_traits<T>::name();
        for (int i = 1; i<argc; ++i) {
            auto tokens = tokenize(std::string(argv[i]),std::regex("="));
            if (tokens[0] == (std::string("--")+searchfor)) {
                IO<T>::from_string(t,tokens[1]);
            }
        }        
    }
};
**/

template<>
struct CommandLine<bool> {
    static std::string help(const std::string& name = "", const std::string& prefix = "") {
        return prefix + "--" + (name.empty()?"bool":name) + " (becomes true) or "
            + "--" + (name.empty()?"bool":name)+"=true/false";      
    } 
    static void load(bool& t, int argc, char** argv, const std::string& name = "") {
        std::string searchfor = name;
        if (searchfor.empty()) searchfor="bool";
        for (int i = 1; i<argc; ++i) {
            auto tokens = tokenize(std::string(argv[i]),std::regex("="));
            if (tokens[0] == (std::string("--")+searchfor)) {
                if ((tokens.size() < 2) || (tokens[1]=="true")) t=true;
                else t=false; 
            }
        }
    }
};

template<typename T>
struct CommandLine<std::optional<T>> {
    static std::string help(const std::string& name = "", const std::string& prefix = "") {
        return prefix + "--" + (name.empty()?type_traits<T>::name():name)
            + "=<" + type_traits<T>::name() + ">";      
    } 
    
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
struct CommandLine<T, std::enable_if_t<is_reflectable_v<T> && (!has_load_commandline_v<T>)>> {
    static std::string help(const std::string& att_name = "", const std::string& prefix = "") {
        std::string sol;
        T t; // This needs to be default constructible, which is problematic but needs to happen anyway in most cases
        t.for_each_attribute([&att_name,&prefix,&sol] (const std::string& name, auto& value) {
            if (att_name.empty()) { //Load directly or by putting the type name in the command line
                sol = sol + CommandLine<std::decay_t<decltype(value)>>::help(name,prefix);
                sol = sol + "   or   " + CommandLine<std::decay_t<decltype(value)>>::help(std::string(type_traits<T>::name())+"-"+name,"");
            } else {
                sol = sol + CommandLine<std::decay_t<decltype(value)>>::help(att_name+"-"+name,prefix);                
            } 
            if constexpr (!is_reflectable_v<std::decay_t<decltype(value)>>) {
                sol = sol + "\n";
            }       
        }); 
        return sol;         
    } 
    
    
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
    static std::string help(const std::string& name = "", const std::string& prefix = "") {
        std::string searchfor = name;
        if (searchfor.empty()) searchfor=type_traits<T>::name();
        if constexpr (has_ostream_operator_v<typename T::value_type>) {
            return prefix + "--" + searchfor + "<"+type_traits<typename T::value_type>::name()+"> <"
                +type_traits<typename T::value_type>::name()+"> <"
                +type_traits<typename T::value_type>::name()+">  ...";
        } else {
            return CommandLine<typename T::value_type>::help(name,prefix);
        } 
    } 
    
    static void load(T& t, int argc, char** argv, const std::string& name = "") {
        std::string searchfor = name;
        if (searchfor.empty()) searchfor=type_traits<T>::name();
        if constexpr (has_ostream_operator_v<typename T::value_type>) {
            for (int i = 1; i<argc; ++i) {
                if (std::string(argv[i])==(std::string("--")+searchfor)) {
                    for (int j = i+1; j<argc;++j) {
                        std::string arg(argv[j]);
                        if ((arg.size() >= 2) && (arg.substr(0,2) == "--")) j=argc;
                        else {
                            typename T::value_type elem;
                            std::stringstream sstr(arg);
                            sstr>>elem;
                            t.push_back(elem);
                        }
                    }
                    i=argc;
                }
            }
        }
        else { 
            //We add only if something is going to be found, and only add one element
            bool do_add = false;
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
    }    
};

template<typename T, std::size_t N>
struct CommandLine<std::array<T,N>> { 
    static std::string help(const std::string& name = "", const std::string& prefix = "") {
        std::string searchfor = name;
        if (searchfor.empty()) searchfor=type_traits<T>::name();
        if constexpr (has_ostream_operator_v<T>) {
            std::string sol = prefix + "--" + searchfor;
            for (std::size_t i = 0; i<N;++i)
                sol = sol + " <"+type_traits<T>::name()+">";
            return sol;
        } else {
            return CommandLine<T>::load(name,prefix);
        } 
    }
    
    static void load(std::array<T,N>& t, int argc, char** argv, const std::string& name = "") {
        std::string searchfor = name;
        if (searchfor.empty()) searchfor=type_traits<T>::name();
        if constexpr (has_ostream_operator_v<T>) {
            for (int i = 1; i<argc; ++i) {
                if (std::string(argv[i])==(std::string("--")+searchfor)) {
                    for (int j = i+1; j<argc;++j) {
                        std::string arg(argv[j]); std::size_t i = 0;
                        if ( (i>=N) ||  ((arg.size() >= 2) && (arg.substr(0,2) == "--")) ) j=argc;
                        else {
                            std::stringstream sstr(arg);
                            sstr>>t[i++];
                        }
                    }
                    i=argc;
                }
            }
        }
        else { 
            //We add only if something is going to be found, and only add one element
            bool do_add = false;
            for (int i = 1; i<argc; ++i) {
                if (std::string(argv[i]).rfind(std::string("--")+searchfor,0) == 0)
                    do_add = true;
            }
            if (do_add) {
                T elem; //Just loads the first element of the collection for now
                CommandLine<typename T::value_type>::load(elem,argc,argv,name);
                if constexpr (is_pimpl_v<typename T::value_type>) {
                    if (elem.impl()) t[0] = elem; 
                } else { 
                    t[0] = elem; 
                }
            }
        }
    }    
};

template<typename T>
struct CommandLine<T, std::enable_if_t<has_load_commandline_v<T>>> {
    static std::string help(const std::string& name = "", const std::string& prefix = "") {
        return T::template help<T>(name,prefix);
    } 
    static void load(T& t, int argc, char** argv, const std::string& name = "") {
        t.load_commandline(argc,argv,name);
    }    
};

inline void exit_if_commandline_help(int argc, char** argv) { 
    for (int i = 1; i<argc; ++i) if (std::string("--help") == argv[i]) exit(0); 
} 

template<typename T>
void load_commandline(T& t, int argc, char** argv, const std::string& name = "") { 
    for (int i = 1; i<argc; ++i) {
        if (std::string("--help") == argv[i])   { 
            std::cout<<CommandLine<T>::help(name, "")<<std::endl;
            //If they ask for help, then print this help and go on (maybe there are more loads)
        } 
    } 
        
    std::ostringstream xml;
    for (int i = 1; i<argc; ++i) {
        std::string arg(argv[i]);
        if ((arg.size()>4) && (arg.substr(arg.length()-4,4) == ".xml")) {
            std::ifstream in(arg);
            if (in.is_open()) {
                xml<<in.rdbuf();
            }
        }
    }
    std::string xmlstring = xml.str();
    if (!xmlstring.empty()) {
        for (int j = 1; j<argc; ++j) {
            auto tokens = tokenize(std::string(argv[j]),std::regex("="));
            replace_string(xmlstring,std::string("$")+tokens[0].substr(2),tokens[1]);
        }
        load_xml(t,xmlstring);
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
