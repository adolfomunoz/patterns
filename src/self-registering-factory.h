#pragma once

#include <unordered_map>
#include <string>
#include <memory>
#include "type-traits.h"
#define NOGDI //Weird windows stuff that we need to do for avoiding a conflicting header
#include "../ext/dylib/include/dylib.hpp"
#undef NOGDI

#if defined(_WIN32) || defined(_WIN64)
#define LIB_EXPORT __declspec(dllexport)
#else
#define LIB_EXPORT
#endif

//For this to work, it should happen that name = pattern::type_traits<Base>::name()
#define PATTERN_EXPORT_REGISTERED \
extern "C" {\
    LIB_EXPORT void* constructor(const std::string& type, const std::string& id) {\
        return pattern::Constructors::constructor(type,id);\
    }\
    LIB_EXPORT void merge(std::unordered_map<std::string,std::unordered_map<std::string,void*>>* that) {\
        pattern::Constructors::merge(that);\
    }\
    LIB_EXPORT std::string types() {\
        return pattern::Constructors::types();\
    }\
    LIB_EXPORT std::list<std::string> constructors_of(const std::string& type) {\
        return pattern::Constructors::constructors_of(type);\
    }\
}\


namespace pattern {

class DynamicLibraryManager {
private:
    static std::unordered_map<std::string,std::unique_ptr<dylib>> loaded;
public:
    static const dylib* load(const std::string& path, const std::string& lib) {
        std::unique_ptr<dylib> lib_ptr;
        if (auto it = loaded.find(path+lib); it == loaded.end()) {
            try {
                lib_ptr = std::make_unique<dylib>(path,lib);
                loaded[path+lib] = std::move(lib_ptr);
            } catch (const dylib::exception& e) {
                lib_ptr = nullptr;
                return nullptr;
            } 
        }
        return loaded[path+lib].get();
    }
};

inline std::unordered_map<std::string,std::unique_ptr<dylib>> DynamicLibraryManager::loaded;

/**
 * This class stores all registered constructors using a dual index.
 * This is only used for:
 *    Dynamic libraries
 *    Debuggers
 */
class Constructors {
public:
    static std::unordered_map<std::string,std::unordered_map<std::string,void*>>* constructors;
    
    template<typename Base>
    static void register_constructor(const std::string& id, void* constructor) {
        (*constructors)[type_traits<Base>::name()][id] = constructor;
    }

    static void merge(std::unordered_map<std::string,std::unordered_map<std::string,void*>>* that) {
        for (auto& [key, value] : (*constructors)) {
            if (auto it = that->find(key); it != that->end()) {
                it->second.insert(value.begin(),value.end());
            }
        }
        that->insert(constructors->begin(),constructors->end());
        delete constructors;
        constructors = that;
    }

    static void* constructor(const std::string& type, const std::string& id) {
        if (auto it = constructors->find(type); it != constructors->end()) {
            if (auto il = it->second.find(id); il != it->second.end()) {
                return il->second;
            }
        }
        return nullptr;
    }

    static std::string types() {
        std::string s;
        for (const auto& [key, value] : (*constructors)) s+=(key+" "); 
        return s;        
    }

    static std::list<std::string> constructors_of(const std::string& type) {
        std::list<std::string> s;
        if (auto it = constructors->find(type); it != constructors->end()) {
            for (const auto& [key, value] : it->second) s.push_back(key); 
        }
        return s;        
    }
}; 

inline std::unordered_map<std::string,std::unordered_map<std::string,void*>>* Constructors::constructors 
    = new std::unordered_map<std::string,std::unordered_map<std::string,void*>>();


// - This works with all types of pointers but I will mostly use shared_ptr
// - Subclasses must have empty constructors. How to change that? We will see...
template<typename Base>
class SelfRegisteringFactory {
    static std::list<std::string> library_paths;
public:
    static void add_library_path(const std::string& lp) { library_paths.push_back(lp); }
    static bool empty_library_paths() { return library_paths.empty(); }
    class Constructor {
    public:
        virtual Base* make() const = 0;    
    };
    
private:
/*  This is not needed anymore
    static std::unordered_map<std::string,Constructor*>& registered_constructors() {
        static std::unordered_map<std::string,Constructor*> r{};
        return r;
    }
*/
    
public:
    static bool register_constructor(const std::string& id, Constructor* constructor) {
        if (id == type_traits<Base>::name()) return false;
        else if (id.empty()) return false;
        void* c = Constructors::constructor(type_traits<Base>::name(),id);
        if (!c) {
            Constructors::register_constructor<Base>(id,constructor);
            return true;
        } else return false;
    }

    static Constructor* constructor(const std::string& id) {
        void* c = Constructors::constructor(type_traits<Base>::name(),id);
        if (c) return reinterpret_cast<Constructor*>(c);
        else return nullptr;
    }

    static Base* make(const std::string& id, const std::list<std::string>& libraries = std::list<std::string>{}) {
        if (library_paths.empty()) add_library_path("./");
        if (id.empty()) {
            #ifdef PATTERN_SHOW_SELF_REGISTERING
            std::cerr<<"[ WARN ] Emtpy id for constructing type "<<type_traits<Base>::name()<<std::endl;
            #endif 
            return nullptr;           
        }
        if (auto c = constructor(id)) {
            #ifdef PATTERN_SHOW_SELF_REGISTERING
            std::cerr<<"[ INFO ] Loading "<<id<<" of type "<<type_traits<Base>::name()<<" from registered"<<std::endl;
            #endif
            return c->make();
        } else for (const auto& library : libraries) {
            for (const auto& path : library_paths) {
                auto lib = DynamicLibraryManager::load(path, library);
                if (lib) try {
                    auto library_constructor = lib->get_function<void*(const std::string&,const std::string&)>("constructor");
                    Constructor* constructor = reinterpret_cast<Constructor*>(library_constructor(type_traits<Base>::name(),id));
                    if (constructor) {
                        auto library_merge = lib->get_function<void(std::unordered_map<std::string,std::unordered_map<std::string,void*>>*)>("merge");
                        if (library_merge) {
                            library_merge(pattern::Constructors::constructors);
                            #ifdef PATTERN_SHOW_SELF_REGISTERING
                            std::cerr<<"[ INFO ] Registering everything from library "<<library<<" on path "<<path<<std::endl;
                            #endif    
                        } else {
                            register_constructor(id,constructor);
                            #ifdef PATTERN_SHOW_SELF_REGISTERING
                            std::cerr<<"[ INFO ] Loading and registering "<<id<<" of type "<<type_traits<Base>::name()<<" from library "<<library<<" on path "<<path<<std::endl;
                            #endif
                        }
                        return constructor->make();
                    } 
                } catch (const dylib::exception& e) {
                    #ifdef PATTERN_SHOW_SELF_REGISTERING
                    std::cerr<<"[ WARN ] Couldn't load "<<id<<" of type "<<type_traits<Base>::name()<<" from library "<<library<<std::endl;
                    std::cerr<<"           -> "<<e.what()<<std::endl;
                    #endif                
                }
            }
        }
        #ifdef PATTERN_SHOW_SELF_REGISTERING
        std::cerr<<"[ WARN ] "<<id<<" of type "<<type_traits<Base>::name()<<" not found anywhere"<<std::endl;
        std::cerr<<"         Tried libraries [ ";
        for (const auto& library : libraries) std::cerr<<library<<" ";
        std::cerr<<"] on library paths [ ";
        for (const auto& path : library_paths) std::cerr<<path<<" ";
        std::cerr<<"]"<<std::endl;
        #endif 
        return nullptr;
    }

    static Base* make(const std::string& id, const std::string& library) {
        return make(id,std::list<std::string>{library});
    }
    
    static std::unique_ptr<Base> make_unique(const std::string& id, const std::list<std::string>& libraries = std::list<std::string>{}) {
        return std::unique_ptr<Base>(make(id,libraries));
    }
    
    static std::shared_ptr<Base> make_shared(const std::string& id, const std::list<std::string>& libraries = std::list<std::string>{}) {
        return std::shared_ptr<Base>(make(id,libraries));
    }
    
    static std::unique_ptr<Base> make_unique(const std::string& id, const std::string& library) {
        return std::unique_ptr<Base>(make(id,library));
    }
    
    static std::shared_ptr<Base> make_shared(const std::string& id, const std::string& library) {
        return std::shared_ptr<Base>(make(id,library));
    }
    
    static std::list<std::string> registered() {
        return Constructors::constructors_of(type_traits<Base>::name());
    }
};

template<typename Base>
std::list<std::string> SelfRegisteringFactory<Base>::library_paths;

// vv Should never happen, it is specialized 
template<typename Self, typename... Bases>
class SelfRegisteringClass  {
    virtual ~SelfRegisteringClass() {} 
};

template<typename Self, typename Base>
class SelfRegisteringClass<Self,Base> {
    class Constructor : public SelfRegisteringFactory<Base>::Constructor {
    public:
        Base* make() const override { 
            if constexpr (std::is_abstract_v<Self>) return nullptr;
            else return new Self; 
        }
    };
public:
    inline static Constructor constructor;
    inline static bool is_registered = 
        SelfRegisteringFactory<Base>::register_constructor((std::is_abstract_v<Self>?std::string():type_traits<Self>::name()),&constructor);
    //We avoid registering abstract classes, which is something that we didn't do before
    #ifdef __GNUC__
    #pragma GCC diagnostic push
    #pragma GCC diagnostic ignored "-Wunused-value"
    #endif
    SelfRegisteringClass() { is_registered; } //This is so the template does not miss it. Cheap trick.
    #ifdef __GNUC__
    #pragma GCC diagnostic pop
    #endif
    virtual ~SelfRegisteringClass() {} 
};

template<typename Self, typename Base, typename... Bases>
class SelfRegisteringClass<Self,Base,Bases...> : public SelfRegisteringClass<Self,Bases...> {
    class Constructor : public SelfRegisteringFactory<Base>::Constructor {
    public:
        Base* make() const override { 
            if constexpr (std::is_abstract_v<Self>) return nullptr;
            else return new Self; 
        }
    };
public:
    inline static Constructor constructor;
    inline static bool is_registered = 
        SelfRegisteringFactory<Base>::register_constructor((std::is_abstract_v<Self>?std::string():type_traits<Self>::name()),&constructor);

    #ifdef __GNUC__   
    #pragma GCC diagnostic push
    #pragma GCC diagnostic ignored "-Wunused-value"
    #endif
    SelfRegisteringClass() { is_registered; } //This is so the template does not miss it. Cheap trick.
    #ifdef __GNUC__
    #pragma GCC diagnostic pop
    #endif
    virtual ~SelfRegisteringClass() {} 
};

}

