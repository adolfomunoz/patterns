#pragma once

#include <unordered_map>
#include <string>
#include <memory>
#include "self-registering-factory.h"
#include "reflect.h"
#include "xml.h"
#include "commandline.h"
#include "pimpl.h"


namespace pattern {
   
class SelfRegisteringReflectableBase {
public: 
//All these three are public because it is really hard to "friend" the corresponding Pimpl class below
//but they should be protected.
    virtual std::string xml_content(const std::string& prefix = "") const = 0;
    virtual void load_content(rapidxml::xml_node<>* node) = 0;
    virtual const char* object_type_name() const = 0;
    virtual void load_commandline_content(int argc, char**argv, const std::string& name) = 0;
};

template<typename Self, typename... Bases>
class SelfRegisteringReflectable : public SelfRegisteringClass<Self, Bases...>, public Reflectable<Self,Bases...> {
public: 
//All these three are public because it is really hard to "friend" the corresponding Pimpl class below
//but they should be protected.
    std::string xml_content(const std::string& prefix = "") const override {
        std::stringstream sstr;
        this->for_each_attribute([&sstr,&prefix] (const std::string& name, const auto& value) {
            sstr<<XML<std::decay_t<decltype(value)>>::get(value,name,prefix+"   ");
        });
        return sstr.str();
    }
    void load_content(rapidxml::xml_node<>* node) override {
        this->for_each_attribute([&node] (const std::string& name, auto& value) {
            XML<std::decay_t<decltype(value)>>::load(value,node,name);
        });          
    }
    const char* object_type_name() const override {
        return type_traits<Self>::name();
    }
    
    virtual void load_commandline_content(int argc, char**argv, const std::string& name) {
        std::cerr<<"Loading content"<<std::endl;
        CommandLine<Self>::load(static_cast<Self&>(*this),argc,argv,name);
    }
};

template<typename Base>
class Pimpl<Base,std::enable_if_t<std::is_base_of_v<SelfRegisteringReflectableBase,Base>>>  : public Pimpl<Base,int> {
public:
    using Pimpl<Base,int>::Pimpl;
    using Pimpl<Base,int>::operator=;

protected:
    std::string xml_content(const std::string& prefix = "") const override {
        return this->impl()->xml_content(prefix);
    }
    void load_content(rapidxml::xml_node<>* node) override {
        this->impl()->load_content(node);
    }
    const char* object_type_name() const override {
        return this->impl()->object_type_name();
    }
    void load_commandline_content(int argc, char**argv, const std::string& name) override {
        this->impl()->load_commandline_content(argc,argv,name);
    }

public:
    std::string xml(const std::string& name = "", const std::string& prefix = "") const {
        std::stringstream sstr;
        sstr<<prefix<<"<"<<type_traits<Base>::name()<<" type=\""<<object_type_name()<<"\">\n";
        sstr<<xml_content(prefix);
        sstr<<"</"<<type_traits<Base>::name()<<">\n";
        return sstr.str();
    }
    
    void load_xml(rapidxml::xml_node<>* node, const std::string& att_name = "") {
        rapidxml::xml_node<>* found = XMLSearch<Base>::find(node,att_name);
        if (found) {
            rapidxml::xml_attribute<>* type = found->first_attribute("type");
            if (type) {
                if ((this->impl()) && (this->object_type_name() == std::string(type->value(),type->value_size()))) load_content(found);
                else {
                    auto ptr = SelfRegisteringFactory<Base>::make_shared(std::string(type->value(),type->value_size()));
                    if (ptr) {
                        (*this) = ptr;
                        load_content(found);
                    }
                }
            }
        }
    }
    
    void load_commandline(int argc, char** argv, const std::string& name = "") {
        std::string type;
        if (name.empty()) { //Load directly or by putting the type name in the command line
            pattern::load_commandline(type,argc,argv,"type");
            pattern::load_commandline(type,argc,argv,std::string(type_traits<Base>::name())+"-type");
        } else {
            pattern::load_commandline(type,argc,argv,name+"-type");                
        }
        if ((this->impl()) && (this->object_type_name() == type)) 
            load_commandline_content(argc,argv,name);
        else {
            auto ptr = SelfRegisteringFactory<Base>::make_shared(type);
            if (ptr) {
                (*this) = ptr;
                this->load_commandline_content(argc,argv,name);
                if (name.empty()) this->load_commandline_content(argc,argv,std::string(type_traits<Base>::name()));
            }
        }
    }    
    
    
/**
    void load_xml(const std::string& xml) { 
        std::string copy(xml);
        rapidxml::xml_document<> doc;
        doc.parse<0>(&copy[0]);
        load_xml(&doc);   
    }
**/
    
};

}

