#pragma once

#include "type-traits.h"
#include <sstream>
#include <fstream>
#include <optional>
#include <algorithm>

namespace pattern {

namespace {
    template <typename T, typename = void>
    struct has_ostream_operator_impl : std::false_type {};

    template <typename T>
    struct has_ostream_operator_impl<T, 
           std::void_t<decltype(std::declval<std::ostream&>()<<std::declval<T>()) >> : std::true_type {};

    template<typename T>
    struct has_ostream_operator {
        static constexpr bool value = has_ostream_operator_impl<T>::value;
    };
    
    template<typename T>
    inline constexpr bool has_ostream_operator_v = has_ostream_operator<T>::value;   

    template <typename T, typename = void>
    struct has_istream_operator_impl : std::false_type {};

    template <typename T>
    struct has_istream_operator_impl<T, 
           std::void_t<decltype(std::declval<std::istream&>()>>std::declval<T&>()) >> : std::true_type {};

    template<typename T>
    struct has_istream_operator {
        static constexpr bool value = has_istream_operator_impl<T>::value;
    };
    
    template<typename T>
    inline constexpr bool has_istream_operator_v = has_istream_operator<T>::value;   

    template<typename T>
    struct has_stream_operators {
        static constexpr bool value = has_ostream_operator<T>::value && has_istream_operator<T>::value;
    };
    
    template<typename T>
    inline constexpr bool has_stream_operators_v = has_stream_operators<T>::value;   


}

template<typename T, typename Enable = void>
struct IO {
    static constexpr bool available = false;
    static std::string to_string(const T& t) { return ""; }
    static bool from_string(T& t, std::string_view s)  { return false; }
};

template<>
struct IO<std::string> {
    static constexpr bool available = true;
    static constexpr const std::string& to_string(const std::string& t) { return t; }
    static bool from_string(std::string& t, std::string_view s) { t=s; return true; }
};

template<typename T>
struct IO<T,std::enable_if_t<has_stream_operators_v<T>>> {
    static constexpr bool available = true;
    static std::string to_string(const T& t) {
        std::stringstream sstr;
        sstr<<t;
        return sstr.str();
    }
    static bool from_string(T& t, std::string_view s) {
        std::istringstream sstr((std::string(s)));
        sstr>>t;
        return true;
    }
};

template<typename T>
struct IO<std::optional<T>> {
    static constexpr bool available = IO<T>::available;
    static std::string to_string(const std::optional<T>& t) { 
        if (t) return IO<T>::to_string(*t); 
        else return "";
    }
    static bool from_string(std::optional<T>& t, std::string_view s) {
        T _t; 
        if (IO<T>::from_string(_t,s)) { t = _t; return true; }
        else { t={}; return false; } 
    }
};

template<typename C>
struct IO<C,std::enable_if_t<is_collection_v<C>>> {
    static constexpr bool available = IO<typename C::value_type>::available;
    static std::string to_string(const C& c) {
        if constexpr (available) {
            std::stringstream sstr; bool first = true;
            for (const auto& item : c) {
                sstr<<IO<typename C::value_type>::to_string(item);
                if (first) { first = false; }
                else sstr<<" ";
            }
            return sstr.str();
        } else return "";
   }
   static bool from_string(C& c, std::string_view s) {
        c.clear();
        if constexpr (has_istream_operator_v<typename C::value_type>) { //For the particular case of istreaming, so we do not depend on separation by spaces
            std::stringstream sstr((std::string(s)));
            typename C::value_type item;
            while (sstr>>item) c.push_back(item);
        } else {
            const auto end = s.end(); auto to = s.begin(); decltype(to) from;
            while((from = std::find_if(to, end, [](char c){ return !std::isspace(c); })) != end) {
                to = std::find_if(from, end, [](char c){ return std::isspace(c); });
                typename C::value_type item;
                if (IO<typename C::value_type>::from_string(item,std::string(from,to))) c.push_back(item);
            }
        }
        return true;
    }
};

}
