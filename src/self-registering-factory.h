#pragma once

#include <unordered_map>
#include <string>
#include <memory>
#include "type-traits.h"

namespace pattern {
   

// - This works with all types of pointers but I will mostly use shared_ptr
// - Subclasses must have empty constructors. How to change that? We will see...
template<typename Base>
class SelfRegisteringFactory {
public:
    class Constructor {
    public:
        virtual Base* make() const = 0;
        std::unique_ptr<Base> make_unique() const {
            return std::unique_ptr<Base>(make());
        }            
        std::shared_ptr<Base> make_shared() const {
            return std::shared_ptr<Base>(make());
        }            
    };
    
private:
    static std::unordered_map<std::string,Constructor*>& registered_constructors() {
        static std::unordered_map<std::string,Constructor*> r{};
        return r;
    }
    
public:
    static bool register_constructor(const std::string& id, Constructor* constructor) {
        if (auto it = registered_constructors().find(id); it == registered_constructors().end()) {
            registered_constructors()[id] = constructor;
            return true;
        } else return false;
    }

    static Base* make(const std::string& id) {
        if (auto it = registered_constructors().find(id); it != registered_constructors().end()) {
            return it->second->make();
        } else return nullptr;
    }
    
    static std::unique_ptr<Base> make_unique(const std::string& id) {
        if (auto it = registered_constructors().find(id); it != registered_constructors().end()) {
            return it->second->make_unique();
        } else return nullptr;
    }
    
    static std::shared_ptr<Base> make_shared(const std::string& id) {
        if (auto it = registered_constructors().find(id); it != registered_constructors().end()) {
            return it->second->make_shared();
        } else return nullptr;
    }
    
    static std::string registered() {
        std::string s;
        for (const auto& [key, value] : registered_constructors()) s+=(key+" "); 
        return s;
    }
};

// vv Should never happen, it is specialized 
template<typename Self, typename... Bases>
class SelfRegisteringClass  {
    virtual ~SelfRegisteringClass() {} 
};

template<typename Self, typename Base>
class SelfRegisteringClass<Self,Base> : public Base {
    class Constructor : public SelfRegisteringFactory<Base>::Constructor {
    public:
        Base* make() const override { return new Self; }
    };
public:
    inline static Constructor constructor;
    inline static bool is_registered = 
        SelfRegisteringFactory<Base>::register_constructor(type_traits<Self>::name(),&constructor);
       
    #pragma GCC diagnostic push
    #pragma GCC diagnostic ignored "-Wunused-value"
    SelfRegisteringClass() { is_registered; } //This is so the template does not miss it. Cheap trick.
    #pragma GCC diagnostic pop
    virtual ~SelfRegisteringClass() {} 
};

template<typename Self, typename Base, typename... Bases>
class SelfRegisteringClass<Self,Base,Bases...> : public Base, public SelfRegisteringClass<Self,Bases...> {
    class Constructor : public SelfRegisteringFactory<Base>::Constructor {
    public:
        Base* make() const override { return new Self; }
    };
public:
    inline static Constructor constructor;
    inline static bool is_registered = 
        SelfRegisteringFactory<Base>::register_constructor(type_traits<Self>::name(),&constructor);
       
    #pragma GCC diagnostic push
    #pragma GCC diagnostic ignored "-Wunused-value"
    SelfRegisteringClass() { is_registered; } //This is so the template does not miss it. Cheap trick.
    #pragma GCC diagnostic pop
    virtual ~SelfRegisteringClass() {} 
};

}

