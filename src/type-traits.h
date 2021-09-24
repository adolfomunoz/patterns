#pragma once

namespace pattern {
    
template<typename C, typename V = void>
struct type_traits_impl {
    static const char* name() { return typeid(C).name(); }  
};

template<typename C>
struct type_traits_impl<C,decltype(C::type_name(),void())> {
    static const char* name() { return C::type_name(); }
};

template<>
struct type_traits_impl<float,void> {
    static const char* name() { return "float"; }
};

template<>
struct type_traits_impl<double,void> {
    static const char* name() { return "float"; }
};

template<>
struct type_traits_impl<int,void> {
    static const char* name() { return "integer"; }
};

template<>
struct type_traits_impl<long,void> {
    static const char* name() { return "integer"; }
};

template<typename C>
struct type_traits : type_traits_impl<std::decay_t<C>> { };



}
