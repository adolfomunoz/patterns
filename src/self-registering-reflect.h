#pragma once

#include <unordered_map>
#include <string>
#include <memory>
#include "self-registering-factory.h"
#include "reflect.h"
#include "xml.h"
#include "pimpl.h"

namespace pattern {
   
class SelfRegisteringReflectableBase {
protected:
    virtual std::string xml_content(const std::string& prefix = "") const = 0;
    virtual void load_content(rapidxml::xml_node<>* node) = 0;
    virtual const char* object_type_name() const = 0;
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
    const char* object_type_name() const override {
        return type_traits<Self>::name();
    }
};

template<typename Base>
class Pimpl<Base,std::enable_if_t<std::is_base_of_v<Base,SelfRegisteringReflectableBase> && std::is_abstract_v<Base>>  : public Pimpl<Base,void> {
public:
    using Pimpl<Base,void>::Pimpl;
    using Pimpl<Base,void>::operator=;
    
    std::string xml(const std::string& name = "", const std::string& prefix = "") {
        std::stringstream sstr;
        sstr<<prefix<<"<"<<type_traits<Base>::name()<<" type=\""<<impl()->object_type_name()<<"\">\n";
        sstr<<impl()->xml_content(prefix);
        sstr<<"</"<<type_traits<Base>::name()<<">\n";
        return sstr.str();
    }
    
    void load_xml(rapidxml::xml_node<>* node, const std::string& att_name = "") {
        rapid_xml::xml_node<>* found = XMLSearch<Base>::find(node,att_name);
        if (found) {
            rapidxml::xml_attribute<>* type = found->first_attribute("type");
            if (type) {
                auto ptr = SelfRegisteringFactory<Base>::make_shared(std::string(type->value(),type->value_size()));
                if (ptr) {
                    (*this) = ptr;
                    impl()->load_content(found);
                }
            }
        }
    }
};

}

