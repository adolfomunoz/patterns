#pragma once
#include <vector>
#include <list>
#include <type_traits>

namespace pattern {

    
template<typename C, typename V = void>
struct type_traits_impl {
    static const char* name() { return typeid(C).name(); }  
};


template<typename C>
struct type_traits_impl<C,std::void_t<decltype(C::type_name())>> {
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

template<>
struct type_traits_impl<unsigned int,void> {
    static const char* name() { return "integer"; }
};

template<>
struct type_traits_impl<unsigned long,void> {
    static const char* name() { return "integer"; }
};

template<>
struct type_traits_impl<std::string,void> {
    static const char* name() { return "string"; }
};

template<typename C>
struct type_traits : public type_traits_impl<std::decay_t<C>> { };

template<typename T>
struct is_collection_impl {
    static constexpr bool value = false;
};

template<typename T>
struct is_collection_impl<std::list<T>> {
    static constexpr bool value = true;
};

template<typename T>
struct is_collection_impl<std::vector<T>> {
    static constexpr bool value = true;
};

template<typename T>
struct is_collection {
    static constexpr bool value = is_collection_impl<std::decay_t<T>>::value;
};

template<typename T>
inline constexpr bool is_collection_v = is_collection<T>::value;

}
