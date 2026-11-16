# Pimpl with reflection and self-registration

A [pimpl](pimpl.md) holds some object behind a common base class. If that base
class is also [reflectable](reflect.md) and
[self-registering](self-registering.md), the wrapper gains three abilities:
it can be **written to XML and loaded from XML as whichever type it holds**,
it can **create that type from its name**, and the types can even live in
**plugins** that are loaded when needed. In
[examples/pimpl-reflect-inheritance](../examples/pimpl-reflect-inheritance),
`Shape` is a pimpl of `ShapeBase`, and `Circle` and `Rectangle` are reflectable
classes derived from `ShapeBase`. The other examples are
[examples/pimpl-reflect-inheritance-init](../examples/pimpl-reflect-inheritance-init),
[examples/pimpl-reflect-inheritance-multilevel](../examples/pimpl-reflect-inheritance-multilevel) and
[examples/pimpl-reflect-plugin](../examples/pimpl-reflect-plugin).

## Why combine them

- **Polymorphic serialization.** A plain pimpl cannot be loaded, since it does
  not know which type to build. Here the XML names the type, and the wrapper
  creates it.
- **Value semantics.** A `Shape`, or a `std::list<Shape>`, is serialized and
  loaded like any other value, with no pointers in user code.
- **No factory code.** Types register themselves, so adding a new shape means
  writing one more class. Nothing central is edited.
- **Extensible at run time.** A type can be compiled in a separate library
  that the program never mentions at build time.

## Using a pimpl with reflection

From [main.cc](../examples/pimpl-reflect-inheritance/main.cc):

```cpp
Shape shape = Circle(2.0);
std::cout << pattern::xml(shape);
```

```xml
<shape type="circle">
    <float name="radius" value="2" />
</shape>
```

The tag is the one of the base class (`shape`), and the attribute `type` tells
which class it holds. The attributes are those of the held object, so no
serialization code was written for `Shape`. Collections work the same way, and
each element states its own type:

```cpp
std::list<Shape> shapes;
shapes.push_back(Circle(2.0));
shapes.push_back(Rectangle(1.0, 2.0));
std::cout << pattern::xml(shapes);
```

### Loading from XML

`load_xml` works on an **empty** wrapper. It reads the `type` attribute, builds
an object of that type through the factory, and loads its attributes:

```cpp
Shape shape;
pattern::load_xml(shape,
    "<shape type=\"rectangle\">\
        <float name=\"height\" value=\"1.0\"/>\
        <float name=\"width\" value=\"2.0\"/>\
     </shape>");
```

- If the wrapper already holds an object of the same type, it is loaded in
  place and attributes missing from the XML keep their value.
- If it holds a different type (or nothing), a new object of the requested
  type replaces it. Loading `<shape type="circle">` into the `Shape` above
  turns it into a `Circle`.
- If the type is not registered, the wrapper is left unchanged.

The tag can also be the name of the derived type (`<rectangle>...</rectangle>`
instead of `<shape type="rectangle">`).

### Creating by name

The wrapper can also be built, or changed, from a type name, with no XML
involved:

```cpp
Shape shape("circle");                 // holds a default Circle
shape.type();                          // "circle"
shape.set_type("rectangle");           // now holds a default Rectangle
shape.set_attribute("width", "3");     // forwarded to the held object
```

| Method | Purpose |
|---|---|
| `Shape(name)` | builds a wrapper holding a default object of the registered type `name` (empty if unknown) |
| `type()` | the name of the held type |
| `set_type(name)` | replaces the held object by a default one of type `name`; unchanged if `name` is unknown |
| `Shape::registered()` | the names of the registered types |
| `get_attribute(name)`, `set_attribute(name, value)` | forwarded to the held object |
| `load_commandline(argc, argv)` | loads the wrapper from the command line, choosing the type with `--type=` |

Everything in the table needs a non-null wrapper, except the constructors,
`set_type`, `registered()` and `load_commandline`. As with any pimpl, calling
a forwarded method on a null wrapper fails an `assert`.

## Writing the classes

From [shape.h](../examples/pimpl-reflect-inheritance/shape.h),
[circle.h](../examples/pimpl-reflect-inheritance/circle.h) and
[rectangle.h](../examples/pimpl-reflect-inheritance/rectangle.h):

```cpp
class ShapeBase : public pattern::SelfRegisteringReflectableBase {
public:
    static const char* type_name() { return "shape"; }
    virtual float area() const = 0;
};

class Shape : public pattern::Pimpl<ShapeBase> {
public:
    using pattern::Pimpl<ShapeBase>::Pimpl;

    float area() const override { return impl()->area(); }
};

class Circle : public pattern::Reflectable<Circle,ShapeBase> {
    float radius;
public:
    Circle(float radius = 1.0f) : radius(radius) {}
    float area() const override { return 3.14159276f*radius*radius; }
    static const char* type_name() { return "circle"; }
    auto reflect_names() const { return std::tuple("radius"); }
    auto reflect() { return std::tie(radius); }
};
```

1. **Base class:** derive it from `pattern::SelfRegisteringReflectableBase`,
   and give it `type_name()` (`"shape"`) and its virtual interface. Deriving
   from this class is what enables the extra features of the pimpl.
2. **Wrapper:** write it as in [Writing a pimpl](pimpl.md#writing-a-pimpl):
   derive from `pattern::Pimpl<Base>`, inherit the constructors, and forward
   each virtual method. Nothing else is forwarded by hand: XML, attributes,
   type creation and the command line are already handled by the library.
3. **Concrete types:** derive from `pattern::Reflectable<Self, Base>`, with
   `reflect()`, `reflect_names()` and `type_name()` as in
   [Writing a reflectable class](reflect.md#writing-a-reflectable-class).
   Because `Base` is self-registering, this also registers the class in the
   factory of `Base` under its `type_name()`. There is no
   `SelfRegisteringClass` to write.
4. **Default constructors:** the factory creates objects with `new Self`, as
   in [Writing a self-registering class](self-registering.md#writing-a-self-registering-class).
   Give every constructor parameter a default value.
5. **Include the classes:** registration happens at start-up, so the header
   of each type must be part of the executable (`main.cc` includes
   `circle.h` and `rectangle.h`), unless the type is a [plugin](#plugins).
6. **Include `patterns.h`:** the layers that join these patterns are defined
   in [self-registering-reflect.h](../src/self-registering-reflect.h), which
   `patterns.h` includes.

## Inheritance

Concrete types can extend other concrete types. From
[examples/pimpl-reflect-inheritance-multilevel](../examples/pimpl-reflect-inheritance-multilevel),
where [circle.h](../examples/pimpl-reflect-inheritance-multilevel/circle.h) and
[regular.h](../examples/pimpl-reflect-inheritance-multilevel/regular.h) define:

```cpp
class Circle : public pattern::Reflectable<Circle,ShapeBase> { /* radius */ };

class Sector : public pattern::Reflectable<Sector,Circle> {
    float angle;
public:
    Sector(float radius = 1.0f, float angle = 1.0f);
    float area() const override { return Circle::area()*angle/(2.0f*3.14159276f); }
    static const char* type_name() { return "sector"; }
    auto reflect_names() const { return std::tuple("angle"); }
    auto reflect() { return std::tie(angle); }
};

template<std::size_t N>
class Regular : public pattern::Reflectable<Regular<N>,ShapeBase> { /* side */ };

class Square : public pattern::Reflectable<Square,Regular<4>> {
public:
    using pattern::Reflectable<Square,Regular<4>>::Reflectable;
    static const char* type_name() { return "square"; }
};
```

```cpp
shapes.push_back(Square(3.0));
shapes.push_back(Sector(2.0, 1.0));
```

```xml
<shape type="sector">
    <float name="angle" value="1" />
    <float name="radius" value="2" />
</shape>
```

- A class is registered in the factory of the **root base** (`ShapeBase`) even
  if it derives from an intermediate class, so `Shape("sector")` and
  `<shape type="sector">` work.
- Attributes are inherited as in [Inheritance](reflect.md#inheritance): the
  attributes of `Sector` come first, followed by those of `Circle`.
- A class that only exists to share code, such as `Regular<N>`, needs no
  `type_name()`. Its subclasses (`Square`, `Pentagon`) provide the names.
- `PATTERN_SHOW_SELF_REGISTERING` is defined for this example in
  [CMakeLists.txt](../CMakeLists.txt), so the registrations are printed on
  `std::cerr` at start-up.

## Initialization after loading

A type that caches values computed from its attributes can use `init()`, as in
[examples/pimpl-reflect-inheritance-init](../examples/pimpl-reflect-inheritance-init):

```cpp
class Rectangle : public pattern::Reflectable<Rectangle,ShapeBase> {
    float width,height;
    float area_;
public:
    void init() { area_ = width*height; }
    Rectangle(float width = 1.0f, float height = 1.0f) : width(width), height(height) { init(); }
    float area() const override { return area_; }
    /* type_name, reflect_names and reflect as usual */
};
```

```cpp
Shape shape;
pattern::load_xml(shape, "<shape type=\"rectangle\">...</shape>");
shape.area();       // uses the loaded width and height: init() was called
```

The wrapper forwards `init()` to the held object, and `load_xml` calls it
once the object has been loaded, so no extra code is needed in `Shape`. This
is the same mechanism as in [Initialization after loading](reflect.md#initialization-after-loading).
A null wrapper ignores `init()`.

## Plugins

The types can also be built in separate dynamic libraries, so the executable
does not know them. From
[examples/pimpl-reflect-plugin](../examples/pimpl-reflect-plugin):

```cpp
// circle.cc, compiled as a shared library
#include "../../patterns.h"
#include "shape.h"

class Circle : public pattern::Reflectable<Circle,ShapeBase> { /* as before */ };

PATTERN_EXPORT_REGISTERED

// main.cc: only includes rectangle.h
Shape shape("circle");
std::cout << shape.area();
pattern::load_xml(shape, "<shape type=\"circle\"><float name=\"radius\" value=\"3.0\"/></shape>");
```

```cmake
add_plugin_on_folder(circle shape examples/pimpl-reflect-plugin/circle.cc)
add_executable(pimpl-reflect-plugin examples/pimpl-reflect-plugin/main.cc)
```

- **Export:** `PATTERN_EXPORT_REGISTERED` is written once per library, as in
  [Plugins](self-registering.md#plugins).
- **Virtual destructor:** `ShapeBase` must declare `virtual ~ShapeBase()` in
  this case, as the object is created in the library and deleted in the
  executable.
- **Automatic search:** unlike the factory alone, the wrapper does not need
  the library name. For a type `name`, it looks for a library called `name`
  (or `shape-name`) in `./` and in `./shape/`, that is, a folder named
  after the base type. `add_plugin_on_folder(circle shape ...)` puts the
  library in exactly that place, `bin/shape/`. Run the executable from `bin`.
- **Hyphens:** a type name like `circle-small` also looks in the libraries
  `circle` and `shape-circle`, so a library can provide several related types.
- **Triggers:** the library is loaded when a type that is not registered is
  requested, through `Shape("circle")`, `set_type` or `load_xml`. Without the
  library, the request is just ignored.
- **Not compile-time:** the executable cannot name `Circle`, so it cannot do
  `shapes.push_back(Circle(2.0))`, but the wrapper can hold one once it is
  created by name or loaded from XML.
- **Tracing:** define `PATTERN_SHOW_SELF_REGISTERING` before including the
  library, as in the example, to see on `std::cerr` which libraries are tried.

## Semantics

- The held object is shared between copies of the wrapper, as in any
  [pimpl](pimpl.md#semantics). Loading into a copy modifies the object seen by
  the other copies, unless the type changes, since a new object is created.
- A wrapper serializes as a `shape` with a `type` attribute. A reflectable
  class serialized by itself uses its own tag (`<circle>`).
- An empty wrapper generates no XML.
- Registration is done at start-up, and names are unique per root base.

## Command line

The same wrapper can be loaded from the command line (`--type=circle
--radius=2`) and can print its own help. See [Command line](commandline.md).
