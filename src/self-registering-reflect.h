#pragma once

#include <unordered_map>
#include <string>
#include <memory>
#include "self-registering-factory.h"
#include "reflect.h"
#include "xml.h"

namespace pattern {
   
template<typename Self>
class SelfRegisteringReflectableBase {
protected:
    virtual std::string xml_content(const std::string& prefix = "") const = 0;
    virtual void load_content(rapidxml::xml_node<>* node) = 0;
};

template<typename Self, typename... Bases>
class SelfRegisteringReflectable : public SelfRegisteringClass<Self, Bases...>, public Reflectable<Self,Bases...> {
protected:
    std::string xml_content(const std::string& prefix = "") const override {
        std::stringstream sstr;
        t.for_each_attribute([&sstr,&prefix] (const std::string& name, const auto& value) {
            sstr<<XML<std::decay_t<decltype(value)>>::get(value,name,prefix+"   ");
        });
    }
    void load_content(rapidxml::xml_node<>* node) override {
        t.for_each_attribute([&node] (const std::string& name, auto& value) {
            XML<std::decay_t<decltype(value)>>::load(value,node,name);
        });          
    }
};


}

