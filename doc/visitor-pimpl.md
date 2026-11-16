# Visitor and Pimpl

A [pimpl](pimpl.md) hides which concrete class it holds, and a
[visitor](visitor.md) finds out which concrete class an object is. Combined,
they let a **value-type wrapper be visited as the object it holds**: the
visitor never sees the wrapper, it receives the `Circle` or `Rectangle` stored
inside. The examples are
[examples/visitor-pimpl](../examples/visitor-pimpl) and
[examples/visitor-generic-pimpl](../examples/visitor-generic-pimpl).

## Why combine them

- **Visitors on values.** `Shape` is used like a plain value (no pointers, no
  `new`), and it still supports `accept`, `apply` and `apply_for`.
- **No forwarding code.** The wrapper does not have to forward `accept`:
  the library does it when the base class is visitable.
- **The wrapper is transparent.** A visitor written for `Circle` and
  `Rectangle` works the same on a `Shape` wrapper as on a `Shape*` to a
  visitable object. `Shape` itself is never listed in a visitor.

## Using a visited pimpl

From [main.cc](../examples/visitor-pimpl/main.cc), with `PrintVisitor` as
defined in [Visitor](visitor.md#visitor-classes-accept):

```cpp
Rectangle rect(2.0, 2.0);

Shape s = rect;                     // stores a copy of the rectangle

PrintVisitor pv;
s.accept(pv);                       // prints: Rectange = 2x2

float area   = s.apply([] (Rectangle& rect) { return rect.area(); },
                       [] (Circle& circ)    { return circ.area(); });   // 4
float radius = s.apply([] (Circle& circ) { return circ.radius(); }, -1.0f);  // -1
```

The generic form from [main.cc](../examples/visitor-generic-pimpl/main.cc)
works the same way:

```cpp
s.apply_for<Circle,Rectangle>([] (const auto& shape) { std::cout<<shape; });
```

The calls are identical to the ones used on a pointer in
[examples/visitor](../examples/visitor), only with `.` instead of `->`.
Assigning another type changes what the next visit sees:

```cpp
s = Circle(1.0);
s.apply([] (Circle& circ) { return circ.radius(); }, -1.0f);    // 1
```

Visiting a null wrapper (`Shape empty;`) fails an `assert`, as any forwarded
method of a pimpl does. Check `is_null()` first when a wrapper may be empty.

## Writing a visitable pimpl

From [shape.h](../examples/visitor-pimpl/shape.h) and
[circle.h](../examples/visitor-pimpl/circle.h):

```cpp
class ShapeBase : public pattern::VisitableBase {
public:
    virtual float area() const = 0;
};

class Shape : public pattern::Pimpl<ShapeBase> {
public:
    using pattern::Pimpl<ShapeBase>::Pimpl;
    using pattern::Pimpl<ShapeBase>::operator=;

    float area() const override { return this->impl()->area(); }
};

class Circle : public pattern::Visitable<Circle,ShapeBase> {
    float radius_;
public:
    Circle(float radius = 1.0f) : radius_(radius) {}
    float radius() const noexcept { return radius_; }
    float area() const override { return 3.14159276f*radius()*radius(); }
};
```

1. **Visitable base:** the base class of the pimpl derives from
   `pattern::VisitableBase` (`ShapeBase`), as in
   [Making a class visitable](visitor.md#making-a-class-visitable).
2. **Visitable concrete types:** each concrete class derives from
   `pattern::Visitable<Self, Base>`, where `Base` is the pimpl base
   (`Visitable<Circle,ShapeBase>`), not the wrapper.
3. **Wrapper:** write it exactly as in
   [Writing a pimpl](pimpl.md#writing-a-pimpl). Forward the ordinary virtual
   methods (`area()`), and **do not** write `accept`: when `Base` derives
   from `VisitableBase`, `Pimpl<Base>` already overrides both `accept`
   methods and forwards them to the held object.
4. **Include `patterns.h`:** the forwarding of `accept` is defined in
   [reflectable-visitable.h](../src/reflectable-visitable.h), which is
   included by `patterns.h`. Including only `pimpl.h` and `visitor.h` does
   not provide it.

## Semantics

- `shape.accept(v)` calls `accept` on the held object, so the visitor
  receives the concrete object stored in the wrapper, not the wrapper.
- The visited object is the one shared by the wrapper, as pimpls hold their
  implementation through a `std::shared_ptr`. A visitor that modifies it
  (for example a lambda taking `Circle&` and changing it) is seen by every copy
  of the wrapper.
- Only the concrete classes are visitable types. The wrapper (`Shape`) does
  not appear in a `Visitor<...>` list nor as a parameter of an `apply`
  callable.
