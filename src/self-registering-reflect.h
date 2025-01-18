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
   
class SelfRegisteringReflectableBase : public XMLAble {
public: 
//All these three are public because it is really hard to "friend" the corresponding Pimpl class below
//but they should be protected.
    virtual void load_content(rapidxml::xml_node<>* found) = 0;
    virtual const char* object_type_name() const = 0;
    //This method is overriden by reflectable classes but we need it here on te base class
    virtual std::string help_from_this(const std::string& name, const std::string& prefix) const {
        return "";
    } 
    virtual void load_commandline_content(int argc, char** argv, const std::string& name) = 0;
    virtual void init() { } //This init method helps other reflectable classes implement their own init methods for second-stage initialization
    
    //This is overriden by reflectable classes but we need it here on the base class
    virtual std::string get_attribute(const std::string& n) const { return ""; }
    //This is overriden by reflectable classes but we need it here on the base class
    virtual void set_attribute(const std::string& n, const std::string& v) { }

    virtual ~SelfRegisteringReflectableBase() {}
};


template<typename Self, typename Base, typename Enable = void>
class CheckedSelfRegistering {};

template<typename Self, typename... Bases>
class CheckedManySelfRegistering {}; //Should never happen

template<typename Self, typename Base>
class CheckedSelfRegistering<Self, Base, 
      typename std::enable_if<!is_reflectable_v<Base> && std::is_abstract_v<Base> && std::is_base_of_v<SelfRegisteringReflectableBase,Base>>::type> :
          public SelfRegisteringClass<Self,Base> {
};

template<typename Self, typename... Bases>
class CheckedSelfRegistering<Self, ReflectableInheritance<Bases...>> : public CheckedManySelfRegistering<Self,Bases...> {};

template<typename Self, typename Base>
class CheckedSelfRegistering<Self, Base, std::enable_if_t<is_reflectable_v<Base> && !std::is_abstract_v<Base> && std::is_base_of_v<SelfRegisteringReflectableBase,Base>>> : public CheckedSelfRegistering<Self,typename Base::FirstBase>, public CheckedSelfRegistering<Self,typename Base::RestOfBases> {
};

template<typename Self, typename Base>
class CheckedSelfRegistering<Self, Base, std::enable_if_t<is_reflectable_v<Base> && std::is_abstract_v<Base>  && std::is_base_of_v<SelfRegisteringReflectableBase,Base>>> : public SelfRegisteringClass<Self,Base>, public CheckedSelfRegistering<Self,typename Base::FirstBase>, public CheckedSelfRegistering<Self,typename Base::RestOfBases> {
};


template<typename Self, typename Base, typename... Bases>
class CheckedManySelfRegistering<Self,Base,Bases...> : public CheckedSelfRegistering<Self,Base>, public CheckedManySelfRegistering<Self,Bases...> {};

template<typename Self, typename Base>
class CheckedManySelfRegistering<Self,Base> : public CheckedSelfRegistering<Self,Base> {};

template<typename... Bases>
struct AnyIsSelfRegistering {
    static constexpr bool value = false;
    
};

template<typename Base, typename... Bases>
struct AnyIsSelfRegistering<Base,Bases...> {
    static constexpr bool value = std::is_base_of<SelfRegisteringReflectableBase,Base>::value ||
        AnyIsSelfRegistering<Bases...>::value;
};

namespace layer {
    constexpr unsigned int self_registering = 1;
};

template<typename Self, typename... Bases>
struct layer_condition<layer::self_registering,Self,Bases...> {
    static constexpr bool value = AnyIsSelfRegistering<Bases...>::value;
};

template<typename Self, typename... Bases>
class ReflectableChecked<layer::self_registering,true,Self,Bases...> : public CheckedManySelfRegistering<Self, Bases...>, public ReflectableImpl<layer::self_registering,Self,Bases...> {
public: 
    using ReflectableImpl<layer::self_registering,Self,Bases...>::ReflectableImpl;
    virtual void load(rapidxml::xml_node<>* node, const std::string& att_name = "") override {
        XML<Self>::load(static_cast<Self&>(*this),node,att_name);
    }
    virtual void load_content(rapidxml::xml_node<>* found) override {
        if constexpr (is_reflectable_v<Self>)
            XML<Self>::load_content(static_cast<Self&>(*this),found);
    }
    virtual bool generates(xml_flag_type flags = 0) const override {
        return XML<Self>::generates(static_cast<const Self&>(*this),flags);
    }
    virtual std::string get_tag(const std::string&  name = "", xml_flag_type flags = 0) const override {
        return XML<Self>::get_tag(static_cast<const Self&>(*this),name,flags);
    } 
    virtual std::string get_attributes(const std::string&  name = "", xml_flag_type flags = 0) const override {
        return XML<Self>::get_attributes(static_cast<const Self&>(*this),name,flags);
    }
    virtual std::string get_content(const std::string&  name = "", const std::string& prefix = "", xml_flag_type flags = 0) const override {
        return XML<Self>::get_content(static_cast<const Self&>(*this),name,prefix,flags);
    }

    const char* object_type_name() const override {
        return type_traits<Self>::name();
    }

    virtual std::string help_from_this(const std::string& name, const std::string& prefix) const override {
        return CommandLine<Self>::help(name,prefix);
    }

    virtual void load_commandline_content(int argc, char**argv, const std::string& name) override {
        CommandLine<Self>::load(static_cast<Self&>(*this),argc,argv,name);
    }     
};


/*
template<typename Self, typename... Bases>
class SelfRegisteringReflectable : public CheckedManySelfRegistering<Self, Bases...>, public Reflectable<Self,Bases...> {
public: 
//All these three are public because it is really hard to "friend" the corresponding Pimpl class below
//but they should be protected.
    std::string xml_content(const std::string& prefix = "", xml_flag_type flags = 0) const override {
        std::stringstream sstr;
        this->for_each_attribute([&sstr,&prefix,&flags] (const std::string& name, const auto& value) {
            if ((!(flags & xml_reflect_attributes_from_stream)) || (
                XML<std::decay_t<decltype(value)>>::get_attribute(value,name,flags).empty()))
                sstr<<XML<std::decay_t<decltype(value)>>::get(value,name,prefix+"   ",flags);
        });        return sstr.str();
    }

    std::string xml_attributes(xml_flag_type flags = 0) const override {
        std::stringstream sstr;
        if (flags & xml_reflect_attributes_from_stream) {
            this->for_each_attribute([&sstr,&flags] (const std::string& name, const auto& value) {
                std::string s = XML<std::decay_t<decltype(value)>>::get_attribute(value,name,flags);
                if (!s.empty()) sstr<<" "<<s;
            });
        }
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
        CommandLine<Self>::load(static_cast<Self&>(*this),argc,argv,name);
    }
};
*/

template<typename Base>
class Pimpl<Base,layer::self_registering,std::enable_if_t<std::is_base_of_v<SelfRegisteringReflectableBase,Base>>>  : public Pimpl<Base,layer::self_registering-1> {
public:
    using Pimpl<Base,layer::self_registering-1>::Pimpl;
    Pimpl(const std::string& type) : Pimpl<Base,layer::self_registering-1>(SelfRegisteringFactory<Base>::make_shared(type)) {}
    Pimpl(const char* type) : Pimpl(std::string(type)) {}
    using Pimpl<Base,layer::self_registering-1>::operator=;

    virtual std::string get_attribute(const std::string& n) const override {
        return this->impl()->get_attribute(n);
    }

    virtual void set_attribute(const std::string& n, const std::string& v) override {
        this->impl()->set_attribute(n,v);
    }

    virtual void load_content(rapidxml::xml_node<>* found) override{
        return this->impl()->load_content(found);
    }
    virtual void load(rapidxml::xml_node<>* node, const std::string& att_name = "") override {
       if (!node) return; //We actually need to load content... maybe get back from top, I dunno
        rapidxml::xml_node<>* found = nullptr;
        std::string loading_type = "";
        for (found = node->first_node(); found; found = found->next_sibling()) {
            rapidxml::xml_attribute<>* name = found->first_attribute("name");
            if ((att_name.empty()) || ((name) && (att_name == std::string_view(name->value(),name->value_size())))) {
                if (type_traits<Base>::name() == std::string_view(found->name(),found->name_size())) { 
                    rapidxml::xml_attribute<>* type = found->first_attribute("type");
                    if (type) loading_type = std::string_view(type->value(),type->value_size());
                } else loading_type = std::string(found->name(),found->name_size());
                if ((!this->is_null()) && (this->object_type_name() == loading_type)) {
                    load_content(found);
                    return;
                } else {
                    auto ptr = SelfRegisteringFactory<Base>::make_shared(loading_type);
                    if (ptr) {
                        (*this) = ptr;
                        load_content(found);
                        return;
                    } 
                }
            }
        }
    }
    virtual bool generates(xml_flag_type flags = 0) const override {
        return ((!this->is_null()) && this->impl()->generates(flags)); 
    }
    virtual std::string get_tag(const std::string& name = "", xml_flag_type flags = 0) const override {
        if (flags & xml_tag_as_derived)
            return this->impl()->object_type_name();
        else
            return type_traits<Base>::name();   
    } 
    virtual std::string get_attributes(const std::string&  name = "", xml_flag_type flags = 0) const override {
        std::stringstream sstr;
        if (!(flags & xml_tag_as_derived)) sstr<<" type=\""<<this->impl()->object_type_name()<<"\" ";
        sstr<<this->impl()->get_attributes(name,flags);
        return sstr.str();
    }
    virtual std::string get_content(const std::string&  name = "", const std::string&  prefix = "", xml_flag_type flags = 0) const override {
        return this->impl()->get_content(name,prefix,flags);
    }

protected:
    const char* object_type_name() const override {
        return this->impl()->object_type_name();
    }
    std::string help_from_this(const std::string& name, const std::string& prefix) const {
        return this->impl()->help_from_this(name,prefix);
    } 
    void load_commandline_content(int argc, char**argv, const std::string& name) override {
        this->impl()->load_commandline_content(argc,argv,name);
    }
public:
    void init() override {
        this->impl()->init();
    }
    std::string type() const { return std::string(object_type_name()); }
    void set_type(const std::string& name) {
        auto ptr = SelfRegisteringFactory<Base>::make_shared(name);
        if (ptr) { (*this) = ptr; }
    }
    
    static bool can_hold_type(const std::string& name) {
        //Warning, this does not seem to work very well
        return SelfRegisteringFactory<Base>::make_shared(name);
    }
      
    static std::list<std::string> registered() {
        return SelfRegisteringFactory<Base>::registered();
    }

    template<typename Self>
    static std::string help(const std::string& name, const std::string& prefix) {
        std::string sol;
        if (name.empty()) {
            sol = prefix + "--type=  or  --"+type_traits<Base>::name()+"-type=(";
        } else {
            sol = prefix + "--"+name+"-type=(";
        } 
        std::list<std::string> types = registered();
        bool first = true;
        for (auto t : types) {
            if (first) first=false;
            else sol = sol+"|";
            sol = sol + t;
        }
        sol = sol+")\n";
        for (auto t : types) {
            sol = sol + prefix + "   " + t + "\n";
            Self helper(t);
            sol = sol + helper.help_from_this(name,prefix + "    | ");
        }   
        return sol;
    } 
        
    void load_commandline(int argc, char** argv, const std::string& name = "") {
        std::string type;
        if (name.empty()) { //Load directly or by putting the type name in the command line
            CommandLine<std::string>::load(type,argc,argv,"type");
            CommandLine<std::string>::load(type,argc,argv,std::string(type_traits<Base>::name())+"-type");
        } else {
            CommandLine<std::string>::load(type,argc,argv,name+"-type");                
        }
        if ((this->impl()) && (type.empty() || (this->object_type_name() == type))) 
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

/*
    void clone_into_self() {
        std::string x = this->xml();
        rapidxml::xml_document<> doc;
        doc.parse<0>(&x[0]);
        load_xml(&doc);
    }  */

    
    
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

