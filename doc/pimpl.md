# Pimpl

A pimpl (pointer to implementation) is a **value-type wrapper around a
polymorphic object**. In [examples/pimpl](../examples/pimpl), `Shape` is a pimpl
of `ShapeBase`: it is used like a plain value, but behaves like whichever
derived class (`Circle`, `Rectangle`) it currently holds.

## Using a pimpl

From [main.cc](../examples/pimpl/main.cc):

```cpp
Shape shape = Circle(2.0);          // built from a derived type
std::cout << shape.area();          // dispatches to Circle::area
shape = Rectangle(1.0, 2.0);        // reassigned to a different type
std::cout << shape.area();          // dispatches to Rectangle::area
```

No pointers or `new` appear in user code. This makes polymorphic values work
where inheritance normally needs pointers:

```cpp
std::list<Shape> shapes;
shapes.push_back(Circle(2.0));
shapes.push_back(Rectangle(1.0, 2.0));

template<typename S>
float total_area(const std::list<S>& shapes);   // needs only S::area()
total_area(shapes);                             // also valid for list<Circle>, list<Rectangle>
```

`Shape` has the same interface as `Circle` and `Rectangle`, so generic code
written against `area()` accepts all three.

### Inspecting the held type

The wrapper offers helpers to find out what it holds:

```cpp
if (shape.instance_of<Circle>()) {
    float r = shape.cast_static<Circle>().radius();
}
else if (shape.instance_of<Rectangle>()) {
    float w = shape.cast_static<Rectangle>().width();
}

Shape empty;
empty.is_null();    // true: default-constructed wrappers hold nothing
```

| Method | Purpose |
|---|---|
| `instance_of<T>()` | `true` if the held object is a `T` (uses `dynamic_cast`) |
| `cast_static<T>()` | reference to the held object as `T`, with no check (use after `instance_of`) |
| `cast_dynamic<T>()` | same, but asserts that the held object is a `T` |
| `is_null()` | `true` if no implementation is held |

Calling `area()` (or any forwarded method) on a null wrapper fails an `assert`.
Check `is_null()` first when a wrapper may be empty.

## Writing a pimpl

From [shape.h](../examples/pimpl/shape.h):

```cpp
class ShapeBase {
public:
    virtual float area() const = 0;
    virtual ~ShapeBase() = default;
};

class Shape : public pattern::Pimpl<ShapeBase> {
public:
    using pattern::Pimpl<ShapeBase>::Pimpl;
    float area() const override { return impl()->area(); }
};
```

1. **Base class:** declare the interface as virtual methods (`ShapeBase`). Give
   it a virtual destructor.
2. **Concrete types:** derive from the base and override the methods
   (`Circle`, `Rectangle`). They can add their own members, such as
   `radius()`, reachable through `cast_static`.
3. **Wrapper:** derive from `pattern::Pimpl<Base>`. This provides `impl()`
   (the held implementation) and the constructors and assignments from any
   type derived from `Base`. Since `Pimpl<Base>` derives from `Base`, the
   wrapper shares the base interface.
4. **Inherit constructors:** `using pattern::Pimpl<Base>::Pimpl;`. Without it
   the wrapper cannot be built from a `Circle` or `Rectangle`.
5. **Forward every virtual method:** override each one in the wrapper as
   `return impl()->method(...)`. Until you do, the wrapper stays abstract.

## Semantics

- The implementation is held through a `std::shared_ptr`, so copying a wrapper
  shares the same implementation object.
- Assigning a concrete value (`shape = Rectangle(...)`) creates a new
  implementation object from it.
