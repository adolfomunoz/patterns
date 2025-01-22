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
#define PATTERN_EXPORT_REGISTERED(Base,name)\
extern "C" {\
    LIB_EXPORT void* constructor_#name(const std::string& id) {\
        return reinterpret_cast<void*> SelfRegisteringFactory<Base>::constructor(id);\
    }\
}\


namespace pattern {
   

// - This works with all types of pointers but I will mostly use shared_ptr
// - Subclasses must have empty constructors. How to change that? We will see...
template<typename Base>
class SelfRegisteringFactory {
public:
    class Constructor {
    public:
        virtual Base* make() const = 0;    
    };
    
private:
    static std::unordered_map<std::string,Constructor*>& registered_constructors() {
        static std::unordered_map<std::string,Constructor*> r{};
        return r;
    }
    
public:
    static bool register_constructor(const std::string& id, Constructor* constructor) {
        if (id == type_traits<Base>::name()) return false;
        else if (auto it = registered_constructors().find(id); it == registered_constructors().end()) {
            registered_constructors()[id] = constructor;
            return true;
        } else {
            return false;
        }
    }

    static Constructor* constructor(const std::string& id) {
        if (auto it = registered_constructors().find(id); it != registered_constructors().end()) 
            return it->second;
        else
            return nullptr;
    }

    static Base* make(const std::string& id) {
        if (auto c = constructor(id)) {
            #ifdef PATTERN_SHOW_SELF_REGISTERING
            std::cerr<<"[ INFO ] Loading "<<id<<" of type "<<type_traits<Base>::name()<<" from current source"<<std::endl;
            #endif
            return c->make();
        } else {
            #ifdef PATTERN_SHOW_SELF_REGISTERING
            std::cerr<<"[ WARN ] "<<id<<" of type "<<type_traits<Base>::name()<<" not found"<<std::endl;
            #endif 
            return nullptr;
        }
    }
    
    static std::unique_ptr<Base> make_unique(const std::string& id) {
        return std::unique_ptr<Base>(make(id));
    }
    
    static std::shared_ptr<Base> make_shared(const std::string& id) {
        return std::shared_ptr<Base>(make(id));
    }
    
    static std::list<std::string> registered() {
        std::list<std::string> s;
        for (const auto& [key, value] : registered_constructors()) s.push_back(key); 
        return s;
    }
};

// vv Should never happen, it is specialized 
template<typename Self, typename... Bases>
class SelfRegisteringClass  {
    virtual ~SelfRegisteringClass() {} 
};

template<typename Self, typename Base>
class SelfRegisteringClass<Self,Base> {
    class Constructor : public SelfRegisteringFactory<Base>::Constructor {
    public:
        Base* make() const override { return new Self; }
    };
public:
    inline static Constructor constructor;
    inline static bool is_registered = 
        SelfRegisteringFactory<Base>::register_constructor(type_traits<Self>::name(),&constructor);

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
        Base* make() const override { return new Self; }
    };
public:
    inline static Constructor constructor;
    inline static bool is_registered = 
        SelfRegisteringFactory<Base>::register_constructor(type_traits<Self>::name(),&constructor);

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

