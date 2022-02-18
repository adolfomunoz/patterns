#pragma once

#include <memory>

namespace pattern {
    
template<typename Base, typename Enable = void>
class Pimpl : public Base {
	std::shared_ptr<Base> base;
public:
    const std::shared_ptr<Base>& impl() const { return base; }

    template<typename T> //Not the most efficient way, I would rather mantain static cast and dynamic cast but that's not possible with virtual base classes.
    T& cast() { return *std::dynamic_pointer_cast<T>(base); }
    template<typename T>
    const T& cast() const { return *std::dynamic_pointer_cast<T>(base); }
    template<typename T>
    bool instance_of() const { return bool(std::dynamic_pointer_cast<T>(base)); }

    
    Pimpl() {}
    
	template<typename I, typename = std::enable_if_t<std::is_base_of_v<Base,I>>>
	Pimpl(I&& impl) : 
        base(std::make_shared<std::decay_t<I>>(std::forward<I>(impl))) {}
	
	Pimpl(const Pimpl& that) : base(that.base) {}
	Pimpl(Pimpl&& that) : base(std::move(that.base)) {}
	
	Pimpl(std::shared_ptr<Base>&& base) :
		base(std::forward<std::shared_ptr<Base>>(base)) {}
	Pimpl(const std::shared_ptr<Base>& base) : base(base){}
   
    template<typename I, typename = std::enable_if_t<std::is_base_of_v<Base,I>>>
    Pimpl& operator=(I&& impl) { this->base = std::make_shared<std::decay_t<I>>(std::forward<I>(impl)); return *this; }
    
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
