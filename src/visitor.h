#pragma once

#include <functional>

namespace pattern {
    
template<typename... Visitables>
class Visitor {
public:
    virtual ~Visitor() {} 
};

template<typename... Visitables>
class ConstVisitor {
public:
    virtual ~ConstVisitor() {} 
};

template<>
class Visitor<> : public ConstVisitor<> {
public:
    virtual ~Visitor() {} 
};


template<typename Visitable>
class ConstVisitorImpl {
public:
    virtual void visit(const Visitable& v) = 0;
};

template<typename Visitable>
class VisitorImpl : public ConstVisitorImpl<Visitable> {
public:
    virtual void visit(Visitable& v) = 0;
    void visit(const Visitable& v) override { return this->visit(const_cast<Visitable&>(v)); }
};

template<typename Visitable, typename... Rest>
class Visitor<Visitable,Rest...> : public VisitorImpl<Visitable>, public Visitor<Rest...> {
};

template<typename Visitable, typename... Rest>
class ConstVisitor<Visitable,Rest...> : public ConstVisitorImpl<Visitable>, public ConstVisitor<Rest...> {
};

template<typename ReturnType, typename... Functions>
class FunctionVisitor : public Visitor<> {
protected:
    ReturnType returned_value_;
public:
    FunctionVisitor(const ReturnType& r = ReturnType()) : returned_value_(r) {}
    FunctionVisitor(ReturnType&& r) : returned_value_(std::forward<ReturnType>(r)) {}
    ReturnType returned_value() const { return returned_value_; }
};

template<typename ReturnType>
class FunctionVisitor<ReturnType,ReturnType> : public FunctionVisitor<ReturnType>{
public:
    FunctionVisitor(const ReturnType& r) : FunctionVisitor<ReturnType>(r) {}
    FunctionVisitor(ReturnType&& r) : FunctionVisitor<ReturnType>(std::forward<ReturnType>(r)) {}  
};

/**
template<typename ReturnType, typename ParameterType>
class FunctionVisitor<ReturnType,std::enable_if_t<std::is_convertible_v<ParameterType,ReturnType>,ParameterType>> : public FunctionVisitor<ReturnType> {
public:
    FunctionVisitor(const ParameterType& r) : FunctionVisitor<ReturnType>(ReturnType(r)) {}
    FunctionVisitor(ParameterType&& r) : FunctionVisitor<ReturnType>(std::forward<ReturnType>(r)) {}  
};
**/

template<>
class FunctionVisitor<void> : public Visitor<> {
public:
    void visit() {};
};

namespace detail {
    template<typename T>
    struct function_traits_impl : public function_traits_impl<decltype(&T::operator())> {
    };

    template<typename R, typename Arg>
    struct function_traits_impl<R(Arg)> {
        using result_type = R;
        using argument_type = Arg;
    };

    // partial specialization for function pointer
    template<typename R, typename Arg>
    struct function_traits_impl<R (*)(Arg)> {
        using result_type = R;
        using argument_type = Arg;
    };

    // partial specialization for std::function
    template<typename R, typename Arg>
    struct function_traits_impl<std::function<R(Arg)>> {
        using result_type = R;
        using argument_type = Arg;
    };

    // partial specialization for pointer-to-member-function operator()
    template<typename T, typename R, typename Arg>
    struct function_traits_impl<R (T::*)(Arg)> {
        using result_type = R;
        using argument_type = Arg;
    };

    // partial specialization for pointer-to-member-function operator()
    template<typename T, typename R, typename Arg>
    struct function_traits_impl<R (T::*)(Arg) const> {
        using result_type = R;
        using argument_type = Arg;
    };

    template<typename T>
    struct function_traits : public function_traits_impl<std::decay_t<T>> {};
};

template<typename ReturnType, typename Function, typename... Functions>
class FunctionVisitor<ReturnType, Function, Functions...> : public VisitorImpl<std::decay_t<typename detail::function_traits<Function>::argument_type>>, public FunctionVisitor<ReturnType,Functions...> {
    Function f;
public:
    FunctionVisitor(Function&& f, Functions&&... fs) : FunctionVisitor<ReturnType,Functions...>(std::forward<Functions>(fs)...), f(std::forward<Function>(f)) {}
    void visit(std::decay_t<typename detail::function_traits<Function>::argument_type>& arg) override {
        this->returned_value_ = f(arg);
    }
};

template<typename Function, typename... Functions>
class FunctionVisitor<void, Function, Functions...> : public VisitorImpl<std::decay_t<typename detail::function_traits<Function>::argument_type>>, public FunctionVisitor<void,Functions...> {
    Function f;
public:
    using FunctionVisitor<void,Functions...>::visit;
    FunctionVisitor(Function&& f, Functions&&... fs) : FunctionVisitor<void,Functions...>(std::forward<Functions>(fs)...), f(std::forward<Function>(f)) {}
    void visit(std::decay_t<typename detail::function_traits<Function>::argument_type>& arg) override {
        f(arg);
    }
};

template<typename ReturnType, typename Function, typename... Types>
class GenericFunctionVisitor {
// We don't do this
};

template<typename ReturnType, typename Function, typename Type>
class GenericFunctionVisitor<ReturnType, Function, Type> : public Visitor<>, public VisitorImpl<Type> {
protected:
    Function function;
    ReturnType returned_type;
public:
    GenericFunctionVisitor(Function&& f) : function(std::move(f)) {}
    GenericFunctionVisitor(const Function& f) : function(f) {}
    void visit(Type& type) override {
        this->returned_type = function(type);
    }
};

template<typename Function, typename Type>
class GenericFunctionVisitor<void, Function, Type> : public Visitor<>, public VisitorImpl<Type> {
protected:
    Function function;
public:
    GenericFunctionVisitor(Function&& f) : function(std::move(f)) {}
    GenericFunctionVisitor(const Function& f) : function(f) {}
    void visit(Type& type) override {
        function(type);
    }
};

template<typename ReturnType, typename Function, typename Type, typename Type2,  typename... Types>
class GenericFunctionVisitor<ReturnType, Function, Type, Type2, Types...> : public VisitorImpl<Type>, public GenericFunctionVisitor<ReturnType, Function, Type2, Types...> {
public:
    using GenericFunctionVisitor<ReturnType, Function, Type2, Types...>::visit;
    using GenericFunctionVisitor<ReturnType, Function, Type2, Types...>::GenericFunctionVisitor;
    void visit(Type& type) override {
        this->returned_type = GenericFunctionVisitor<Function,Type2,Types...>::function(type);
    }
};

template<typename Function, typename Type, typename Type2, typename... Types>
class GenericFunctionVisitor<void, Function, Type, Type2, Types...> : public VisitorImpl<Type>, public GenericFunctionVisitor<void, Function, Type2, Types...> {
public:
    using GenericFunctionVisitor<void, Function, Type2, Types...>::visit;
    using GenericFunctionVisitor<void, Function, Type2, Types...>::GenericFunctionVisitor;
    void visit(Type& type) override {
        GenericFunctionVisitor<void,Function,Type2,Types...>::function(type);
    }
};




class VisitableBase {
public:
   virtual void accept(Visitor<>& v) = 0;
   virtual void accept(ConstVisitor<>& v) const = 0;

   template<typename Function, typename... Functions>
   typename detail::function_traits<Function>::result_type apply(Function&& f, Functions&&... fs) {
        FunctionVisitor<typename detail::function_traits<Function>::result_type,Function,Functions...> visitor(std::forward<Function>(f), std::forward<Functions>(fs)...);
        this->accept(visitor);
        if constexpr (!std::is_same_v<typename detail::function_traits<Function>::result_type,void>) return visitor.returned_value();
   }

   template<typename Function, typename... Functions>
   typename detail::function_traits<Function>::result_type apply(Function&& f, Functions&&... fs) const {
        FunctionVisitor<typename detail::function_traits<Function>::result_type,Function,Functions...> visitor(std::forward<Function>(f), std::forward<Functions>(fs)...);
        this->accept(visitor);
        if constexpr (!std::is_same_v<typename detail::function_traits<Function>::result_type,void>) return visitor.returned_value();
   }

   template<typename Type, typename... Types, typename Function>
   auto apply_for(Function&& f) {
        using ReturnType = std::decay_t<decltype(f(std::declval<Type>()))>;
        GenericFunctionVisitor<ReturnType,Function,Type,Types...> visitor(std::forward<Function>(f));
        this->accept(visitor);
        if constexpr (!std::is_same_v<ReturnType,void>) return visitor.returned_value();
   }

   template<typename Type, typename... Types, typename Function>
   auto apply_for(Function&& f) const {
        using ReturnType = std::decay_t<decltype(f(std::declval<Type>()))>;
        GenericFunctionVisitor<ReturnType,Function,Type,Types...> visitor(std::forward<Function>(f));
        this->accept(visitor);
        if constexpr (!std::is_same_v<ReturnType,void>) return visitor.returned_value();
   }
};

#define VISITABLE_METHODS(Self) \
    void accept(VisitorImpl<Self>& v) { v.visit(static_cast<Self&>(*this)); }\
    void accept(ConstVisitorImpl<Self>& v) const { v.visit(static_cast<const Self&>(*this)); }\
    void accept(Visitor<>& v) override {\
        VisitorImpl<Self>* specific = dynamic_cast<VisitorImpl<Self>*>(&v);\
        if (specific) this->accept(*specific);\
    }\
    void accept(ConstVisitor<>& v) const override {\
        ConstVisitorImpl<Self>* specific = dynamic_cast<ConstVisitorImpl<Self>*>(&v);\
        if (specific) this->accept(*specific);\
    }

/* This collides with the useful apply definition. 
 * It is more efficient but neglects the usefulness of the visitor by hiding it.
   template<typename Function, typename... Functions>\
   typename detail::function_traits<Function>::result_type apply(Function&& f, Functions&&... fs) {\
        if constexpr (std::is_same_v<Self,typename detail::function_traits<Function>::argument_type>) return f(static_cast<Self&>(*this));\
        else return apply(std::forward<Functions>(fs)...);\
   }\
   template<typename Function, typename... Functions>\
   typename detail::function_traits<Function>::result_type apply(Function&& f, Functions&&... fs) const {\
        if constexpr (std::is_same_v<Self,typename detail::function_traits<Function>::argument_type>) return f(static_cast<Self&>(*this));\
        else return apply(std::forward<Functions>(fs)...);\
   }\
   template<typename Function>\
   typename detail::function_traits<Function>::result_type apply(Function&& f, const typename detail::function_traits<Function>::result_type& def) {\
        if constexpr (std::is_same_v<Self,typename detail::function_traits<Function>::argument_type>) return f(static_cast<Self&>(*this));\
        else return def;\
   }\
   template<typename Function>\
   typename detail::function_traits<Function>::result_type apply(Function&& f, const typename detail::function_traits<Function>::result_type& def) const {\
        if constexpr (std::is_same_v<Self,typename detail::function_traits<Function>::argument_type>) return f(static_cast<Self&>(*this));\
        else return def;\
   }\
   template<typename Function>\
   typename detail::function_traits<Function>::result_type apply(Function&& f) {\
        if constexpr (std::is_same_v<Self,typename detail::function_traits<Function>::argument_type>) return f(static_cast<Self&>(*this));\
        else return typename detail::function_traits<Function>::result_type();\
   }\
   template<typename Function>\
   typename detail::function_traits<Function>::result_type apply(Function&& f) const {\
        if constexpr (std::is_same_v<Self,typename detail::function_traits<Function>::argument_type>) return f(static_cast<Self&>(*this));\
        else return typename detail::function_traits<Function>::result_type();\
   }\
*/
        
template<typename Self, typename Derived>
class Visitable : public Derived {
public:
    using Derived::apply;
    VISITABLE_METHODS(Self)
};

}