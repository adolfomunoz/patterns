#pragma once
#include <tuple>
#include <iostream>
#include <string>
#include "type-traits.h"
#include "xml.h"

namespace pattern {

namespace {

template <std::size_t Index, typename Tuple, typename Functor>
auto tuple_at_const(const Tuple& tpl, const Functor& func) -> void {
    const auto& v = std::get<Index>(tpl);
    func(v);
};

template <std::size_t Index, typename Tuple, typename Functor>
auto tuple_at(Tuple& tpl, const Functor& func) -> void {
    auto& v = std::get<Index>(tpl);
    func(v);
};

template <typename Tuple, typename Functor, std::size_t Index = 0>
auto tuple_for_each_const(const Tuple& tpl, const Functor& f) -> void {
    constexpr auto tuple_size = std::tuple_size_v<Tuple>;
    if constexpr(Index < tuple_size) {
        tuple_at<Index>(tpl,f);
        tuple_for_each_const<Tuple, Functor, Index+1>(tpl, f);
    }
}

template <typename Tuple, typename Functor, std::size_t Index = 0>
auto tuple_for_each(Tuple& tpl, const Functor& f) -> void {
    constexpr auto tuple_size = std::tuple_size_v<Tuple>;
    if constexpr(Index < tuple_size) {
        tuple_at<Index>(tpl,f);
        tuple_for_each<Tuple, Functor, Index+1>(tpl, f);
    }
}

template <std::size_t Index, typename Refl>
const char* attribute_name(const Refl& r, 
        std::enable_if_t<std::tuple_size_v<decltype(std::declval<Refl>().reflect_names())> <= Index, int> sfinae = 0) {
    return "";   
}

template <std::size_t Index, typename Refl>
const char* attribute_name(const Refl& r, 
        std::enable_if_t<! (std::tuple_size_v<decltype(std::declval<Refl>().reflect_names())> <= Index), int> sfinae = 0) {
    return std::get<Index>(r.reflect_names());;   
}

}

/**
 * Curiously Recurring Template Pattern
 * Self should have:
 *     - The method "reflect()" returning a tuple with references to all meaningful attributes.
 *     - Optionally: the method "attribute_names()" returning a tuple with strings that have attribute names.
 *     - Optionally: the static method "type_name()" returning a string with the name of the type.
 **/
template<typename Self>
class Reflectable {
//    vv This does not work yet. Should find a way to make it work.
//    static_assert(std::tuple_size_v<decltype(std::declval<Self>().reflect())> > 0, "A reflectable class needs to have the reflect() method returning a tuple");
public:
    bool operator==(const Self& that) const { return static_cast<const Self*>(this)->reflect()==that.reflect(); }
    bool operator!=(const Self& that) const { return static_cast<const Self*>(this)->reflect()!=that.reflect(); }
    bool operator<(const Self& that)  const { return static_cast<const Self*>(this)->reflect()<that.reflect();  }
    bool operator>(const Self& that)  const { return static_cast<const Self*>(this)->reflect()>that.reflect();  }
    bool operator<=(const Self& that) const { return static_cast<const Self*>(this)->reflect()<=that.reflect(); }
    bool operator>=(const Self& that) const { return static_cast<const Self*>(this)->reflect()>=that.reflect(); }
    
    auto const_reflect() const { 
        return const_cast<Self*>(static_cast<const Self*>(this))->reflect(); 
    }
    
    //Functor is a function with two parameters: std::string (name) and T& attribute.
    template<typename Functor, std::size_t Index = 0>
    void for_each_attribute(const Functor& f) {
        constexpr std::size_t n = std::tuple_size_v<decltype(const_reflect())>;        
        if constexpr (Index < n) {
            f(attribute_name<Index>(static_cast<Self&>(*this)),
              std::get<Index>(static_cast<Self*>(this)->reflect()));
            for_each_attribute<Functor,Index+1>(f);
        }
    }
    
    //Functor is a function with two parameters: std::string (name) and const T& attribute.
    template<typename Functor, std::size_t Index = 0>
    void for_each_attribute(const Functor& f) const {
        constexpr std::size_t n = std::tuple_size_v<decltype(const_reflect())>;        
        if constexpr (Index < n) {
            f(attribute_name<Index>(static_cast<const Self&>(*this)),
              std::get<Index>(const_reflect()));
            for_each_attribute<Functor,Index+1>(f);
        }
    }
    
protected:  
    template<std::size_t Index>
    std::string xml_attributes(const std::string& prefix = "") const {
        std::stringstream sstr;
        for_each_attribute([&sstr,&prefix] (const std::string& name, const auto& attribute) {
            sstr<<pattern::xml(attribute,name,prefix);
        });
        return sstr.str();
    }

public:
    std::string xml(const std::string& name = "", const std::string& prefix = "") const {
        std::stringstream sstr;
        sstr<<prefix<<"<"<<type_traits<Self>::name();
        if (name != "") sstr<<"name=\""<<name<<"\" ";
        sstr<<">\n";
        sstr<<xml_attributes<0>(prefix+"   ");
        sstr<<prefix<<"</"<<type_traits<Self>::name()<<">\n";
        return sstr.str();
    }   
};

namespace {
    template<typename T>
    struct is_reflectable {
        static constexpr bool value = std::is_base_of_v<Reflectable<T>,T>;
    };

    template<typename T>
    inline constexpr bool is_reflectable_v = is_reflectable<T>::value; 
}

template<typename T>
auto operator<<(std::ostream& os, const T& v) -> std::enable_if_t<is_reflectable_v<T>, std::ostream&> {
    v.for_each_attribute([&os] (const std::string& name, const auto& value) {
        //if (!name.empty()) os<<name<<"=";
        os<<value<<" ";
    });
    return os;
}

template<typename T>
auto operator>>(std::istream& is, T& v) -> std::enable_if_t<is_reflectable_v<T>, std::istream&> {
    v.for_each_attribute([&is] (const std::string& name, auto& value) {
        //if (!name.empty()) os<<name<<"=";
        is>>value;
    });
    return is;
}
    



}
