#pragma once

#include <memory>
#include <cassert>
#include "layers.h"

namespace pattern {

template<typename Base, unsigned int Layer = layer::total, typename Enable = void>
class Pimpl : public Pimpl<Base,Layer-1> {
public:
    using Pimpl<Base,Layer-1>::Pimpl;
    using Pimpl<Base,Layer-1>::operator=;
};


    
template<typename Base>
class Pimpl<Base,layer::basic> : public Base {
	std::shared_ptr<Base> base;
public:
    bool is_null() const noexcept { return base == nullptr; } 
    const std::shared_ptr<Base>& impl() const { 
        assert(base != nullptr); //We make sure that it is not a null ptr
        return base; 
    }

    template<typename T> 
    T& cast_static() { return static_cast<T&>(*base); }
    template<typename T>
    const T& cast_static() const { return static_cast<const T&>(*base); }
    template<typename T>
    bool instance_of() const { return bool(std::dynamic_pointer_cast<T>(base)); }
    template<typename T> 
    T& cast_dynamic() { 
        assert(this->instance_of<T>());
        return *std::dynamic_pointer_cast<T>(base); 
    }
    template<typename T>
    const T& cast_dynamic() const { 
        assert(this->instance_of<T>());
        return *std::dynamic_pointer_cast<T>(base); 
    }
 
    
    Pimpl():base(nullptr) {}
    
	template<typename I, typename = std::enable_if_t<std::is_base_of_v<Base,I>>>
	Pimpl(I&& impl) : 
        base(std::make_shared<std::decay_t<I>>(std::forward<I>(impl))) {}
	template<typename I, typename = std::enable_if_t<std::is_base_of_v<Base,I>>>
	Pimpl(const I& impl) : 
        base(std::make_shared<std::decay_t<I>>(impl)) {}
	
	Pimpl(const Pimpl& that) : base(that.base) {}
	Pimpl(Pimpl&& that) : base(std::move(that.base)) {}
	
	Pimpl(std::shared_ptr<Base>&& base) :
		base(std::forward<std::shared_ptr<Base>>(base)) {}
	Pimpl(const std::shared_ptr<Base>& base) : base(base){}
   
    template<typename I, typename = std::enable_if_t<std::is_base_of_v<Base,I>>>
    Pimpl& operator=(I&& impl) { this->base = std::make_shared<std::decay_t<I>>(std::forward<I>(impl)); return *this; }
    template<typename I, typename = std::enable_if_t<std::is_base_of_v<Base,I>>>
    Pimpl& operator=(const I& impl) { this->base = std::make_shared<std::decay_t<I>>(impl); return *this; }
    
	Pimpl& operator=(const Pimpl& that) { base = that.base; return *this; }
	Pimpl& operator=(Pimpl&& that) { base = std::move(that.base); return *this; }
	
	Pimpl& operator=(std::shared_ptr<Base>&& base) { this->base = std::forward<std::shared_ptr<Base>>(base); return *this; }
	Pimpl& operator=(const std::shared_ptr<Base>& base) { this->base = base; return *this; }
};

// SFINAE test
template <typename T>
class is_pimpl
{
private:
    typedef char YesType[1];
    typedef char NoType[2];

    template <typename C> static YesType& test( decltype(&C::impl) ) ;
    template <typename C> static NoType& test(...);


public:
    static constexpr bool value = sizeof(test<T>(0)) == sizeof(YesType);
};


template<typename T>
inline constexpr bool is_pimpl_v = is_pimpl<T>::value;

    
}
