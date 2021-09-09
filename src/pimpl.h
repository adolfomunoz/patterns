#pragma once

#include <memory>

namespace pattern {
    
template<typename Base>
class Pimpl : public Base {
	std::shared_ptr<Base> base;
protected:
    const std::shared_ptr<Base>& impl() const { return base; }
    
public:
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
    
}