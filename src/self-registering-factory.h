#pragma once

#include <map>
#include <string>
#include <memory>

namespace pattern {

// - This works with shared_ptr (which I mostly use) although it would be trivial 
//   to define a new version based on unique_ptr
// - Subclasses must have empty constructors. How to change that? We will see...
template<typename Base>
class SelfRegisteringFactory {
    inline static std::map<std::string,std::shared_ptr<Base>(*)()> registered_constructors;
public:
    static bool register_constructor(const std::string& id, std::shared_ptr<Base>(*constructor)()) {
        if (auto it = registered_constructors.find(id); it == registered_constructors.end()) { 
            registered_constructors[id] = constructor;
            return true;
        } else return false;
    }

    static std::shared_ptr<Base> create(const std::string& id) {
        if (auto it = registered_constructors.find(id); it == registered_constructors.end()) {
            return it->second();
        } else return nullptr;
    }
};

namespace {

template<typename C, bool has_class_name = std::is_same_v<decltype(C::type_name()),std::string>>
struct registration {
    static std::string name() { return C::type_name(); }
};

template<typename C>
struct registration<C,false> {
    static std::string name() { return typeid(C).name(); }  
};

}

template<typename Derived, typename Base>
class SelfRegisteringClass : public Base {
    inline static bool is_registered = SelfRegisteringFactory<Base>::register_constructor(registration<Derived>::name(),std::make_shared<Derived>);
};








};
