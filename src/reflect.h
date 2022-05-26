#pragma once
#include <tuple>
#include <iostream>
#include <string>
#include "type-traits.h"
#include "layers.h"

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

/**
 * Bases are the superclasses of the base class.
 * This class is the spezialization for zero base classes.
 * It will be specialized
 * It provides the method "for_all_base_casses" which statically traverses all the base classes.
 **/
template<typename Self, typename... Bases>
class ReflectableInheritance {
public:
    using FirstBase = void;
    using RestOfBases = int;

    /**
     * F is a function that can take a single parameter for any of the type of the bases.
     **/
    template<typename F>
    void for_all_base_classes(const F& f) {}
    
    /**
     * F is a function that can take a single parameter for any of the type of the bases.
     **/
    template<typename F>
    void for_all_base_classes(const F& f) const {}       
};  

template<typename Self, typename Base, typename... Bases>
class ReflectableInheritance<Self,Base,Bases...>: public Base, public ReflectableInheritance<Self,Bases...> {
public:
    using FirstBase = Base;
    using RestOfBases = ReflectableInheritance<Bases...>;

    /**
     * F is a function that can take a single parameter for any of the type of the bases.
     **/
    template<typename F>
    void for_all_base_classes(const F& f) {
        f(static_cast<Base&>(*this));
        ReflectableInheritance<Self,Bases...>::for_all_base_classes(f);
    }
    
    /**
     * F is a function that can take a single parameter for any of the type of the bases.
     **/
    template<typename F>
    void for_all_base_classes(const F& f) const {
        f(static_cast<const Base&>(*this));
        ReflectableInheritance<Self,Bases...>::for_all_base_classes(f);
    }    
};

template<typename Self, typename Base>
class ReflectableInheritance<Self,Base>: public Base {
public:
    using FirstBase = Base;
    using RestOfBases = void;

   /**
     * F is a function that can take a single parameter for any of the type of the bases.
     **/
    template<typename F>
    void for_all_base_classes(const F& f) {
        f(static_cast<Base&>(*this));
    }
    
    /**
     * F is a function that can take a single parameter for any of the type of the bases.
     **/
    template<typename F>
    void for_all_base_classes(const F& f) const {
        f(static_cast<const Base&>(*this));
    }    
};


template<unsigned int Layer, typename Self, typename... Bases>
struct layer_condition {
    static constexpr bool value = false;
};

template<typename Self, typename... Bases>
void print_conditions() {
    std::cerr<<"[ ";
    if (layer_condition<0,Self,Bases...>::value) std::cerr<<"0 ";
    if (layer_condition<1,Self,Bases...>::value) std::cerr<<"1 ";
    if (layer_condition<2,Self,Bases...>::value) std::cerr<<"2 ";
    if (layer_condition<3,Self,Bases...>::value) std::cerr<<"3 ";
    if (layer_condition<4,Self,Bases...>::value) std::cerr<<"4 ";
    std::cerr<<"]"<<std::endl;
}

template<unsigned int Layer, bool Condition, typename Self, typename... Bases>
class ReflectableChecked;

template<unsigned int Layer, typename Self, typename... Bases>
class ReflectableImpl : public ReflectableChecked<Layer-1,layer_condition<Layer-1,Self,Bases...>::value,Self,Bases...> {};

template<unsigned int Layer, bool Condition, typename Self, typename... Bases>
class ReflectableChecked : public ReflectableImpl<Layer,Self,Bases...> {};

template<typename Self, typename... Bases>
class ReflectableImpl<layer::basic,Self,Bases...> : public ReflectableInheritance<Self,Bases...> { 
public: 
    //vv The reflect method is here so we have an empty implementation by default but Self should
    //   have its own reflect method for its attributes.
    std::tuple<> reflect() { return std::tuple<>(); }
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
        } else if constexpr (Index == n) { //At the end we deal with the base classes
            this->for_all_base_classes([&f] (auto& base) {
                if constexpr (is_reflectable_v<std::decay_t<decltype(base)>>) 
                    base.for_each_attribute(f);
            });
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
        } else if constexpr (Index == n) { //At the end we deal with the base classes
            this->for_all_base_classes([&f] (const auto& base) {
                if constexpr (is_reflectable_v<std::decay_t<decltype(base)>>) 
                    base.for_each_attribute(f);
            });
        }
    }
};

/**
 * Curiously Recurring Template Pattern
 * Self should have:
 *     - The method "reflect()" returning a tuple with references to all meaningful attributes.
 *     - Optionally: the method "reflect_names()" returning a tuple with strings that have attribute names.
 *     - Optionally: the static method "type_name()" returning a string with the name of the type.
 * Bases... are base classes of the reflectable class. It is automatically detected whether they 
 *     are reflectable themselves for inheritance of the reflectable properties as well.
 **/
template<typename Self, typename... Bases>
class Reflectable : public ReflectableImpl<layer::total,Self,Bases...> { };

template<typename C>
struct has_init {
private:
    template<typename T>
    static constexpr auto check(T*)
    -> typename std::is_same<void,decltype(((T*)(nullptr))->init())>::type;  // attempt to call it and see if the return type is correct

    template<typename>
    static constexpr std::false_type check(...);

    typedef decltype(check<C>(0)) type;

public:
    static constexpr bool value = type::value;
};

template<typename T>
inline constexpr bool has_init_v = has_init<T>::value; 



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
    if constexpr (has_init_v<T>) v.init();
    return is;
}
    



}
