#pragma once
#include <tuple>
#include <iostream>

namespace pattern {

namespace {

template <std::size_t Index, typename Tuple, typename Functor>
auto tuple_at(const Tuple& tpl, const Functor& func) -> void {
    const auto& v = std::get<Index>(tpl);
    func(v);
};

template <typename Tuple, typename Functor, std::size_t Index = 0>
auto tuple_for_each(const Tuple& tpl, const Functor& f) -> void {
    constexpr auto tuple_size = std::tuple_size_v<Tuple>;
    if constexpr(Index < tuple_size) {
        tuple_at<Index>(tpl,f);
        tuple_for_each<Tuple, Functor, Index+1>(tpl, f);
    }
}
}

/**
 * Curiously Recurring Template Pattern
 * Self should have:
 *     - The method "reflect()" returning a tuple with references to all meaningful attributes.
 *     - Optionally: the method "attribute_names()" returning a tuple with strings that have attribute names.
 *     - Optionally: the static method "type_name()" returning a string with the name of the type.
 **/
template<typename Self>
class Reflectable {
//    vv This does not work yet. Should find a way to make it work.
//    static_assert(std::tuple_size_v<decltype(std::declval<Self>().reflect())> > 0, "A reflectable class needs to have the reflect() method returning a tuple");
public:
    bool operator==(const Self& that) const { return static_cast<const Self*>(this)->reflect()==that.reflect(); }
    bool operator!=(const Self& that) const { return static_cast<const Self*>(this)->reflect()!=that.reflect(); }
    bool operator<(const Self& that)  const { return static_cast<const Self*>(this)->reflect()<that.reflect();  }
    bool operator>(const Self& that)  const { return static_cast<const Self*>(this)->reflect()>that.reflect();  }
    bool operator<=(const Self& that) const { return static_cast<const Self*>(this)->reflect()<=that.reflect(); }
    bool operator>=(const Self& that) const { return static_cast<const Self*>(this)->reflect()>=that.reflect(); }
};

namespace {
    template<typename T>
    struct is_reflectable {
        static constexpr bool value = std::is_base_of_v<Reflectable<T>,T>;
    };

    template<typename T>
    inline constexpr bool is_reflectable_v = is_reflectable<T>::value; 
}

template<typename T>
auto operator<<(std::ostream& os, const T& v) -> std::enable_if_t<is_reflectable_v<T>, std::ostream&> {
    tuple_for_each(v.reflect(), [&os](const auto& attribute) { os << attribute << " "; });
    return os;
}
    



}
