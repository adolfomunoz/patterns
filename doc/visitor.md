# Visitor

A visitor is an **operation defined outside of a class hierarchy** that
behaves differently for each concrete class it is applied to. In
[examples/visitor](../examples/visitor), `Shape` is a hierarchy with two
concrete classes (`Circle`, `Rectangle`), and a visitor prints the dimensions
of whichever one it receives, even though `Shape` has no `print` method and
`Circle::radius()` does not exist in `Shape`. The examples are
[examples/visitor](../examples/visitor) and
[examples/visitor-generic](../examples/visitor-generic).

## Why use a visitor

- **New operations without touching the classes.** Printing, exporting or
  measuring a shape is written once, in the visitor, rather than as another
  virtual method on `Shape`, `Circle` and `Rectangle`.
- **Access to the concrete type.** 

The visitor receives a `Circle&` or a
  `Rectangle&`, so it can use members that are not part of the base
  interface (`radius()`, `width()`) without `dynamic_cast` or `if` chains in
  user code.
- **Related code stays together.** The handling of all types lives in one
  place instead of being spread over the hierarchy.

The library removes the usual cost of the pattern: the `accept` methods and
the visitor interfaces are generated for you, and visitors can be written as
plain lambdas.

## Using a visitor

From [main.cc](../examples/visitor/main.cc), where `s` is a `Shape*` that
points to a `Rectangle`:

```cpp
Shape* s = &rect;

PrintVisitor pv;
s->accept(pv);                      // prints: Rectange = 2x2
```

No `dynamic_cast` appears in user code: `accept` selects the `visit` that
matches the real type of the object.

The library offers three ways to visit an object:

| Method | Purpose |
|---|---|
| `accept(visitor)` | visit with a visitor class derived from `Visitor<...>` or `ConstVisitor<...>` |
| `apply(f1, f2, ..., [default])` | visit with one lambda (or function) per type, and get back a result |
| `apply_for<T1, T2, ...>(f)` | visit with one generic lambda that is used for every listed type |

All of them are members of `pattern::VisitableBase`, so they are available on
any visitable class and through a pointer or reference to the base.

### Visitor classes: `accept`

A visitor class derives from `pattern::Visitor<Types...>` (may modify the
visited object) or `pattern::ConstVisitor<Types...>` (only reads it), and
implements one `visit` for each type listed:

```cpp
class PrintVisitor : public pattern::ConstVisitor<Circle,Rectangle> {
    void visit(const Rectangle& rect) {
        std::cout<<"Rectange = "<<rect.width()<<"x"<<rect.height()<<std::endl;
    }
    void visit(const Circle& circ) {
        std::cout<<"Circle   = "<<circ.radius()<<std::endl;
    }
};
```

Use this form when the visitor has state, is reused, or has several
operations that share code.

- A `Visitor<Types...>` declares `visit(T&)` for each type. It can be passed
  to `accept` on both const and non-const objects.
- A `ConstVisitor<Types...>` declares `visit(const T&)` for each type. It can
  only be passed to `accept` on a const object (or a const reference).
- If the object is of a type that is not in the list of the visitor, `accept`
  does nothing.

### Lambdas: `apply`

`apply` receives one callable per type. Each callable takes a single argument,
and its parameter type selects which objects it handles. The callable that
matches the real type of the object is invoked, and its result is returned:

```cpp
float a = s->apply([] (Rectangle& rect) { return rect.area(); },
                   [] (Circle& circ)    { return circ.area(); });
// a == 4 for the Rectangle above
```

The return type of `apply` is the return type of the first callable, so all
callables should return the same type. It can also be `void`.

When the object is of none of the listed types, `apply` returns a default
value. By default this is `ReturnType()`; a different default can be given as
the **last argument**:

```cpp
float r = s->apply([] (Circle& circ) { return circ.radius(); }, -1.0f);
// r == -1 because s holds a Rectangle
```

This is useful for queries that apply only to some types without writing a
visitor class for them.

### Generic lambdas: `apply_for`

When the same code works for several types, `apply_for` avoids repeating the
lambda. The types are listed as template arguments, and a generic lambda
handles all of them. From [main.cc](../examples/visitor-generic/main.cc):

```cpp
s->apply_for<Circle,Rectangle>([] (const auto& shape) { std::cout<<shape; });
```

Here `operator<<` is defined for both `Circle` and `Rectangle`, and the lambda
is instantiated once for each. Types that are not listed are ignored.
`apply_for` is intended for operations that do not return a value (printing,
accumulating into a captured variable, and so on).

## Making a class visitable

From [shape.h](../examples/visitor/shape.h) and
[circle.h](../examples/visitor/circle.h):

```cpp
#include "../../patterns.h"

class Shape : public pattern::VisitableBase {
public:
    virtual float area() const = 0;
};

class Circle : public pattern::Visitable<Circle,Shape> {
    float radius_;
public:
    Circle(float radius = 1.0f) : radius_(radius) {}
    float radius() const noexcept { return radius_; }
    float area() const override { return 3.14159276f*radius()*radius(); }
};
```

1. **Base class:** derive the root of the hierarchy from
   `pattern::VisitableBase` (`Shape`). This provides `accept`, `apply` and
   `apply_for`.
2. **Concrete types:** for each class that a visitor should be able to tell
   apart, derive from `pattern::Visitable<Self, Base>`, where `Self` is the
   class itself and `Base` is the class it extends (`Visitable<Circle,Shape>`,
   `Visitable<Rectangle,Shape>`). `Visitable` generates the `accept` methods
   that call back the right `visit`.
3. **Visitors:** write them as classes derived from `Visitor` or
   `ConstVisitor`, or use `apply` / `apply_for` with lambdas. Nothing else
   needs to be added to the visited classes, no matter how many operations are
   written afterwards.

A class that derives from `Visitable<Self, Base>` can itself be the `Base` of
another one, so a hierarchy with several levels can be made visitable.

## Semantics

- The visited type is resolved at run time with a `dynamic_cast` on the
  visitor, so visiting works through a pointer or reference to the base class
  (`Shape*`), and an object is visited as its most-derived `Visitable` type.
- Visiting is **type-driven**: a type that does not appear in the visitor
  (or in any callable given to `apply`) is silently skipped.
- Calling a visitor with a `Visitor` or lambdas taking non-const references on
  a const object is allowed: constness is cast away on the `visit` call, so
  such visitors should not modify the object.

## Combining with other patterns

Visitable classes can be stored in a [pimpl](pimpl.md) wrapper, which gives
value semantics to the hierarchy and forwards `accept`, `apply` and
`apply_for` to the held object. See [Visitor and Pimpl](visitor-pimpl.md).
