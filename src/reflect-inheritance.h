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


}
