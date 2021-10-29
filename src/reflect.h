#pragma once
#include <tuple>
#include <iostream>
#include <string>
#include "type-traits.h"

namespace pattern {

namespace {

template <typename> struct is_tuple: std::false_type {};
template <typename ...T> struct is_tuple<std::tuple<T...>>: std::true_type {};

template<typename C>
struct has_reflect_names {
private:
    template<typename T>
    static constexpr auto check(T*)
    -> typename is_tuple<decltype(std::declval<T>().reflect_names())>::type;  // attempt to call it and see if the return type is correct

    template<typename>
    static constexpr std::false_type check(...);

    typedef decltype(check<C>(0)) type;

public:
    static constexpr bool value = type::value;
};

template<typename T>
inline constexpr bool has_reflect_names_v = has_reflect_names<T>::value; 

template <std::size_t Index, typename Refl>
const char* attribute_name(const Refl& r,std::enable_if_t<!has_reflect_names_v<Refl>, int> sfinae = 0) {
    return "";   
}

template <std::size_t Index, typename Refl>
const char* attribute_name(const Refl& r,std::enable_if_t<has_reflect_names_v<Refl>, int> sfinae = 0) {
    if constexpr (std::tuple_size_v<decltype(std::declval<Refl>().reflect_names())> > Index)
        return std::get<Index>(r.reflect_names());
    else
        return "";
}

}

/**
 * Curiously Recurring Template Pattern
 * Self should have:
 *     - The method "reflect()" returning a tuple with references to all meaningful attributes.
 *     - Optionally: the method "reflect_names()" returning a tuple with strings that have attribute names.
 *     - Optionally: the static method "type_name()" returning a string with the name of the type.
 **/
template<typename Self>
class Reflectable {
//    vv This does not work yet. Should find a way to make it work.
//    static_assert(std::tuple_size_v<decltype(std::declval<Self>().reflect())> > 0, "A reflectable class needs to have the reflect() method returning a tuple");
public:
//We remove these operators here here because it might get messy when inheritance later on...
/**
    bool operator==(const Self& that) const { return static_cast<const Self*>(this)->reflect()==that.reflect(); }
    bool operator!=(const Self& that) const { return static_cast<const Self*>(this)->reflect()!=that.reflect(); }
    bool operator<(const Self& that)  const { return static_cast<const Self*>(this)->reflect()<that.reflect();  }
    bool operator>(const Self& that)  const { return static_cast<const Self*>(this)->reflect()>that.reflect();  }
    bool operator<=(const Self& that) const { return static_cast<const Self*>(this)->reflect()<=that.reflect(); }
    bool operator>=(const Self& that) const { return static_cast<const Self*>(this)->reflect()>=that.reflect(); }
**/
    
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
};

/**
template<typename T>
struct is_reflectable {
    static constexpr bool value = std::is_base_of_v<Reflectable<T>,T>;
};
**/

template<typename C>
struct is_reflectable {
private:
    template<typename T>
    static constexpr auto check(T*)
    -> typename is_tuple<decltype(std::declval<T>().reflect())>::type;  // attempt to call it and see if the return type is correct

    template<typename>
    static constexpr std::false_type check(...);

    typedef decltype(check<C>(0)) type;

public:
    static constexpr bool value = type::value;
};

template<typename T>
inline constexpr bool is_reflectable_v = is_reflectable<T>::value; 

template<typename T>
auto reflect(T& t, std::enable_if_t<is_reflectable_v<T>,int> dummy = 0) {
    return t.reflect();
}

template<typename T>
auto reflect(const T& t, std::enable_if_t<is_reflectable_v<T>,int> dummy = 0) {
    return const_cast<T&>(t).reflect();
}

//Functor is a function with two parameters: std::string (name) and auto& attribute.
template<typename T, typename Functor, std::size_t Index = 0>
void for_each_attribute(T& t, const Functor& f, std::enable_if_t<is_reflectable_v<T>,int> dummy = 0) {
    constexpr std::size_t n = std::tuple_size_v<decltype(reflect(t))>;        
    if constexpr (Index < n) {
        f(attribute_name<Index>(t),std::get<Index>(reflect(t)));
        for_each_attribute<T,Functor,Index+1>(t,f);
    }
}

//Functor is a function with two parameters: std::string (name) and const auto& attribute.
template<typename T, typename Functor, std::size_t Index = 0>
void for_each_attribute(const T& t, const Functor& f, std::enable_if_t<is_reflectable_v<T>,int> dummy = 0) {
    constexpr std::size_t n = std::tuple_size_v<decltype(reflect(t))>;        
    if constexpr (Index < n) {
        f(attribute_name<Index>(t),std::get<Index>(reflect(t)));
        for_each_attribute<T,Functor,Index+1>(t,f);
    }
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
