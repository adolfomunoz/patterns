#pragma once

namespace pattern {
    
template<typename C, typename V = void>
struct type_traits {
    static std::string name() { return typeid(C).name(); }  
};

template<typename C>
struct type_traits<C,decltype(C::type_name(),void())> {
    static std::string name() { return C::type_name(); }
};



}
