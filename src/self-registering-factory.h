#pragma once

#include <unordered_map>
#include <string>
#include <memory>
#include "type-traits.h"

namespace pattern {
   

// - This works with shared_ptr (which I mostly use) although it would be trivial 
//   to define a new version based on unique_ptr
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

template<typename Derived, typename Base>
class SelfRegisteringClass : public Base {
    class Constructor : public SelfRegisteringFactory<Base>::Constructor {
    public:
        Base* make() const override { return new Derived; }
    };
public:
    inline static Constructor constructor;
    inline static bool is_registered = 
        SelfRegisteringFactory<Base>::register_constructor(type_traits<Derived>::name(),&constructor);
        
    SelfRegisteringClass() { is_registered; } //This is so the template does not miss it. Cheap trick.
    virtual ~SelfRegisteringClass() {} 
};

