# Reflectable

A `Reflectable` class **exposes its attributes** to generic code. C++ has no
built-in reflection, so the class lists its attributes once, and the library
uses that list to iterate over them, read and write them by name, and
serialize them to and from XML. The examples are
[examples/reflect](../examples/reflect),
[examples/reflect-inheritance](../examples/reflect-inheritance) and
[examples/reflect-init](../examples/reflect-init).

## Using a reflectable class

From [main.cc](../examples/reflect/main.cc), with `Rectangle` and
`Transformed` defined as shown in [Writing a reflectable class](#writing-a-reflectable-class):

```cpp
#include "../../src/xml.h"

Rectangle rect(1.0, 2.0);
std::cout << pattern::xml(rect);
```

```xml
<rectangle>
    <float name="width" value="1" />
    <float name="height" value="2" />
</rectangle>
```

No serialization code was written for `Rectangle`. Reflectable objects can
contain other reflectable objects, and they are written recursively:

```cpp
Transformed g(3, -2, Rectangle(5, 5));
std::cout << pattern::xml(g);
```

```xml
<transformed>
    <float name="x" value="3" />
    <float name="y" value="-2" />
    <rectangle>
            <float name="width" value="5" />
            <float name="height" value="5" />
    </rectangle>
</transformed>
```

### Loading from XML

`pattern::load_xml(object, text)` fills an existing object. Attributes can be
written as child nodes or as XML attributes of the node, and the order does
not matter:

```cpp
pattern::load_xml(rect,
    "<rectangle>\
        <float name=\"height\" value=\"1.0\"/>\
        <float name=\"width\" value=\"3.0\"/>\
     </rectangle>");
pattern::load_xml(rect, "<rectangle width=\"1.0\" height=\"3.0\"/>");
```

Attributes that do not appear in the XML keep their current value. Loading
`<transformed><rectangle><float name="height" value="1.0"/></rectangle></transformed>`
into `g` only changes the height of the rectangle.

Plain values and collections work as well, without being reflectable:

```cpp
int i;
pattern::load_xml(i, "<integer value=\"3\"/>");

std::list<float> l{1.0f, 2.0f, 3.0f};
std::cout << pattern::xml(l);        // <float value="1" /> <float value="2" /> ...
pattern::load_xml(l, "<float value=\"-1.0\" /><float value=\"-2.0\" /><float value=\"-3.0\" />");
```

| Function | Purpose |
|---|---|
| `pattern::xml(x)` | XML text of `x` (reflectable, collection or basic value) |
| `pattern::load_xml(x, text)` | loads `x` from XML text |
| `pattern::load_xml_file(x, path)` | same, from a file; returns `false` if it cannot be opened |
| `pattern::make_from_xml<T>(text)` | builds a `T` (default constructible) from XML text |

Other serializations, such as the one for the command line in
[commandline.h](../src/commandline.h) (see [Command line](commandline.md)),
build on the same mechanism.

### Visiting the attributes

Generic code can go through the attributes without XML:

```cpp
rect.for_each_attribute([] (const std::string& name, auto& value) {
    std::cout << name << " = " << value << "\n";     // width = 1, height = 2
});

rect.set_attribute("width", "4");                    // by name, from a string
std::string w = rect.get_attribute("width");         // "4"
```

| Method | Purpose |
|---|---|
| `for_each_attribute(f)` | calls `f(name, attribute)` for every attribute, including those of reflectable base classes. `f` is generic since each attribute has its own type; the attribute is `const` in a `const` object |
| `get_attribute(name)` | the attribute as a string, or an empty string if it does not exist |
| `set_attribute(name, value)` | sets the attribute from a string; unknown names are ignored |

`get_attribute` and `set_attribute` only work on attributes that can be
converted to and from text (numbers, `std::string`, `bool`, `std::optional`
and collections of these, or any type with `<<` and `>>` operators). Other
attributes are skipped. Nameless attributes have the name `""`.

## Advantages

- **Write attributes once:** a single `reflect()` method replaces separate
  serialization, deserialization and printing code, which would otherwise get
  out of sync as attributes are added.
- **Composable:** reflectable classes can contain other reflectable classes,
  collections and basic types, and are serialized as a whole.
- **Generic code:** algorithms can work on any reflectable type without
  knowing it, for example loading configuration or building editors.
- **Non-intrusive layout:** attributes stay ordinary private members. There
  are no macros nor a separate description language.
- **Works with the rest of the library:** reflectable classes can be
  [self-registering](self-registering.md) and used from a
  [pimpl](pimpl.md) to be created from XML by type name.

## Writing a reflectable class

From [rectangle.h](../examples/reflect/rectangle.h) and
[transformed.h](../examples/reflect/transformed.h):

```cpp
class Rectangle : public pattern::Reflectable<Rectangle> {
    float width, height;
public:
    Rectangle(float width = 1.0f, float height = 1.0f) : width(width), height(height) {}
    float area() const { return width * height; }

    auto reflect() { return std::tie(width, height); }
    auto reflect_names() const { return std::tuple("width", "height"); }
    static const char* type_name() { return "rectangle"; }
};

class Transformed : public pattern::Reflectable<Transformed> {
    float x, y;
    Rectangle rect;
public:
    Transformed(float x, float y, const Rectangle& r) : x(x), y(y), rect(r) {}
    auto reflect() { return std::tie(x, y, rect); }
    auto reflect_names() const { return std::tuple("x", "y"); }
    static const char* type_name() { return "transformed"; }
};
```

1. **Derive from `pattern::Reflectable<Self>`** (curiously recurring template
   pattern): `Self` is the class being defined.
2. **`reflect()`** returns a `std::tuple` of *references* to the attributes,
   which is what `std::tie` builds. Their order is the order in which they
   are visited. It is not `const`: it gives write access, which loading needs.
3. **`reflect_names()`** (optional) returns a tuple of names, in the same
   order. Names identify the attributes when loading and in `get_attribute` and
   `set_attribute`. It can have fewer names than `reflect()`: the remaining
   attributes are nameless. In `Transformed`, `rect` has no name, since the
   `<rectangle>` tag already identifies it.
4. **`type_name()`** (recommended) is a static method returning the XML tag
   of the class. Without it the compiler's mangled name is used, with a
   deprecation warning.
5. **Attribute types** must be basic types, collections, or reflectable
   themselves. For each one, XML uses the tag given by its own `type_name()`
   (`float`, `integer`, `rectangle`...).
6. **Default constructor:** needed by `make_from_xml` and by collections of
   the class, which create an empty element and then load it.

If a class has no `reflect()`, it has no attributes of its own (see below).

## Inheritance

Reflectable classes can also declare their base classes, as in
[rectangle.h](../examples/reflect-inheritance/rectangle.h):

```cpp
class Area {
public:
    virtual float area() const = 0;
    static const char* type_name() { return "area"; }
};

class Perimeter { /* the same, with perimeter() */ };

class Rectangle : public pattern::Reflectable<Rectangle, Area, Perimeter> {
    float width, height;
public:
    Rectangle(float width = 1.0f, float height = 1.0f) : width(width), height(height) {}
    float area() const override { return width * height; }
    float perimeter() const override { return 2.0f * (width + height); }
};
```

`Reflectable<Self, Bases...>` derives from every base in the list, so the
bases do not have to be listed again in the class declaration. Constructors
of the bases are inherited by default.

- **Base classes known at compile time:** `for_all_base_classes(f)` calls `f`
  with each base class (as a reference to it):

  ```cpp
  rect.for_all_base_classes([] (const auto& a) {
      std::cout << pattern::type_traits<decltype(a)>::name() << '\n';   // area, perimeter
  });
  ```

- **Inherited attributes:** a base that is itself reflectable (its own
  `Reflectable<...>`) contributes its attributes to `for_each_attribute`, and
  therefore to `xml`, `load_xml`, `get_attribute` and `set_attribute`. After
  the attributes of the class itself, those of each base are visited in the
  order of the list. Non-reflectable bases, like `Area` and `Perimeter` above,
  are skipped.
- **Interfaces:** a base can be a pure interface. Virtual methods are
  overridden in the derived class as usual.

## Initialization after loading

Loading writes the attributes directly, bypassing the constructor. If the
class caches values derived from them, those values become stale. For this a
class can define an `init()` method, as in
[rectangle.h](../examples/reflect-init/rectangle.h):

```cpp
class Rectangle : public pattern::Reflectable<Rectangle> {
    float width, height;
    float area_;
public:
    void init() { area_ = width * height; }
    Rectangle(float width = 1.0f, float height = 1.0f) : width(width), height(height) { init(); }
    float area() const { return area_; }
    auto reflect() { return std::tie(width, height); }
    auto reflect_names() const { return std::tuple("width", "height"); }
    static const char* type_name() { return "rectangle"; }
};
```

```cpp
Rectangle rect(1.0, 2.0);                                      // Area = 2
pattern::load_xml(rect, "<rectangle width=\"1.0\" height=\"3.0\"/>");
rect.area();                                                   // 3: load_xml called init()
```

- `init()` must be a `void` method taking no arguments. It is detected
  automatically; no registration is needed.
- `load_xml` (and `load_xml_file`, `make_from_xml`) call `init()` once the
  whole object has been loaded.
- Calling it from the constructor too, as above, keeps constructed and loaded
  objects consistent.
- Cached values are not attributes: leave them out of `reflect()` so they are
  not serialized.

## Semantics

- Attributes are accessed through the references returned by `reflect()`, so
  loading modifies the object in place.
- Loading does not reset the object: attributes missing from the XML keep
  their value. Collections are the exception: they are cleared and then
  filled with the loaded elements.
- Everything is resolved at compile time, except `get_attribute` and
  `set_attribute`, which are virtual and compare names at run time.

## Combining with other patterns

A reflectable class that derives from a self-registering base can be held by a
[pimpl](pimpl.md), which is then serialized and loaded as the type it holds.
See [Pimpl with reflection and self-registration](pimpl-reflect.md).
