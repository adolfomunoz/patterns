#pragma once
#include <iostream>
#include <string>
#include "type-traits.h"

namespace pattern {

/**
 * Base and Bases are the superclasses of the base class.
 **/
template<typename Base, typename ... Bases>
class ReflectableInheritance : public Base, public ReflectableInheritance<Bases...> {
public:
    /**
     * F is a function that can take a single parameter for any of the type of the bases.
     **/
    template<typename F>
    void for_all_base_classes(const F& f) {
        f(static_cast<Base&>(*this));
        ReflectableInheritance<Bases...>::for_all_base_classes(f);
    }
    
    /**
     * F is a function that can take a single parameter for any of the type of the bases.
     **/
    template<typename F>
    void for_all_base_classes(const F& f) const {
        f(static_cast<const Base&>(*this));
        ReflectableInheritance<Bases...>::for_all_base_classes(f);
    }    
};
    
/**
 * Base and Bases are the superclasses of the base class.
 **/
template<typename Base>
class ReflectableInheritance<Base> : public Base {
public:
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

namespace {
template <template <typename...> class BaseTemplate, typename Derived, typename TCheck = void>
struct test_base_template;

template <template <typename...> class BaseTemplate, typename Derived>
using is_base_template_of = typename test_base_template<BaseTemplate, Derived>::is_base;

//Derive - is a class. Let inherit from Derive, so it can cast to its protected parents
template <template <typename...> class BaseTemplate, typename Derived>
struct test_base_template<BaseTemplate, Derived, std::enable_if_t<std::is_class_v<Derived>>> : Derived
{
    template<typename...T>
    static constexpr std::true_type test(BaseTemplate<T...> *);
    static constexpr std::false_type test(...);
    using is_base = decltype(test((test_base_template *) nullptr));
};

//Derive - is not a class, so it is always false_type
template <template <typename...> class BaseTemplate, typename Derived>
struct test_base_template<BaseTemplate, Derived, std::enable_if_t<!std::is_class_v<Derived>>>
{
    using is_base = std::false_type;
};

template<typename T>
struct has_reflectable_inheritance_impl {
    static constexpr bool value = is_base_template_of<ReflectableInheritance,T>::value;
};
}

template<typename T>
using has_reflectable_inheritance = has_reflectable_inheritance_impl<std::decay_t<T>>;


template<typename T>
inline constexpr bool has_reflectable_inheritance_v = has_reflectable_inheritance<T>::value; 



}
