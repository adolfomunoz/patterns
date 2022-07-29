#pragma once

#include "reflect.h"
#include "visitor.h"
#include "pimpl.h"

namespace pattern {


template<typename... Bases>
struct AnyIsVisitable {
    static constexpr bool value = false;
    
};

template<typename Base, typename... Bases>
struct AnyIsVisitable<Base,Bases...> {
    static constexpr bool value = std::is_base_of<VisitableBase,Base>::value ||
        AnyIsVisitable<Bases...>::value;
};

namespace layer {
    constexpr unsigned int visitable = 2;
};

template<typename Self, typename... Bases>
struct layer_condition<layer::visitable,Self,Bases...> {
    static constexpr bool value = AnyIsVisitable<Bases...>::value;
};

template<typename Self, typename... Bases>
class ReflectableChecked<layer::visitable,true,Self,Bases...> : public ReflectableImpl<layer::visitable,Self,Bases...> {
public:
    using ReflectableImpl<layer::visitable,Self,Bases...>::ReflectableImpl;
    VISITABLE_METHODS(Self)
};

template<typename Base>
class Pimpl<Base,layer::visitable,std::enable_if_t<std::is_base_of_v<VisitableBase,Base>>>  : public Pimpl<Base,layer::visitable-1> {
public:
    using Pimpl<Base,layer::visitable-1>::Pimpl;
    using Pimpl<Base,layer::visitable-1>::operator=;

    void accept(Visitor<>& v) override {
        return this->impl()->accept(v);
    }
    void accept(ConstVisitor<>& v) const override {
        return this->impl()->accept(v);
    }
};

}




