# Self-registering classes

A `SelfRegisteringClass` is a class that **registers itself in a factory**
just by being defined. Once registered, objects of that class can be created
from a plain string (its name), without the creating code knowing the class.
In [examples/self-registering-factory](../examples/self-registering-factory),
`Circle` and `Rectangle` register themselves as `Shape`s, and `main.cc` builds
them from a name given on the command line.

## Using a self-registering class

From [main.cc](../examples/self-registering-factory/main.cc):

```cpp
#include "circle.h"
#include "rectangle.h"

if (auto shape = pattern::SelfRegisteringFactory<Shape>::make_unique(argv[1])) {
    std::cout << shape->area() << std::endl;   // ./self-registering-factory rectangle
}

for (auto r : pattern::SelfRegisteringFactory<Shape>::registered())
    std::cout << r << " ";                     // lists "circle rectangle" (in no particular order)
```

`main.cc` never mentions `Circle` or `Rectangle`: it only names `Shape`. Adding
a new shape means writing one more class; no factory, `if` chain or `switch`
has to be edited.

| Method of `SelfRegisteringFactory<Base>` | Purpose |
|---|---|
| `make_unique(name)` | builds a `std::unique_ptr<Base>`, or a null pointer if `name` is unknown |
| `make_shared(name)` | same, with a `std::shared_ptr<Base>` |
| `make(name)` | same, with a raw `Base*` owned by the caller |
| `registered()` | the names registered for `Base` |

Always check the result: an unknown (or empty) name gives a null pointer.

## Advantages

- **Decoupling:** the code that creates objects depends only on the base
  class and a string, not on the concrete types.
- **Open for extension:** a new type is added by defining it. Nothing central
  needs to be modified, so independent modules can contribute types.
- **Data-driven creation:** the type can come from a command line, a
  configuration file or user input, since it is just a string.
- **Discoverability:** `registered()` lists the available types, useful for
  help messages or validation.
- **Plugins:** types can live in dynamic libraries that are loaded on demand
  (see [Plugins](#plugins)).
- **Little boilerplate:** registration is one entry in the base class list.

## Writing a self-registering class

From [shape.h](../examples/self-registering-factory/shape.h) and
[rectangle.h](../examples/self-registering-factory/rectangle.h):

```cpp
class Shape {
public:
    static const char* type_name() { return "shape"; }
    virtual ~Shape() = default;
    virtual float area() const = 0;
};

class Rectangle : public pattern::SelfRegisteringClass<Rectangle, Shape>, public Shape {
    float width, height;
public:
    Rectangle(float width = 1.0f, float height = 1.0f) : width(width), height(height) {}
    float area() const override { return width * height; }

    static const char* type_name() { return "rectangle"; }
};
```

1. **Base class with a virtual destructor:** the factory returns a `Base*`
   that is later deleted through that type. Without `virtual ~Base()`
   the destructor of the derived class is not called, and when the base
   subobject is not at the start of the object (as in the example, where
   `SelfRegisteringClass` comes first) the `delete` crashes with
   `free(): invalid pointer`.
2. **Type names:** both the base and the derived class provide
   `static const char* type_name()`. The derived name is the string used to
   create objects (`"rectangle"`); the base name identifies the factory.
   Without `type_name()` the compiler's mangled name is used and a deprecation
   warning is emitted.
3. **Inherit from `pattern::SelfRegisteringClass<Self, Base>`:** `Self` is
   the class being defined and `Base` is the type the factory returns. This
   is what performs the registration. Inherit publicly from `Base` as well.
4. **Default constructor:** the factory creates objects with `new Self`, so
   `Self` must be default constructible. Giving every constructor parameter a
   default value, as `Rectangle` does, is enough.
5. **Include the class definition:** registration happens during program
   start-up, so the header must be part of the executable (or library) that
   uses the factory. `main.cc` includes `circle.h` and `rectangle.h` even
   though it does not use them directly.

### Several bases

`SelfRegisteringClass` accepts more than one base. The class is registered in
the factory of each of them:

```cpp
class Circle : public pattern::SelfRegisteringClass<Circle, Shape, Drawable>, public Shape, public Drawable { ... };
```

## Semantics

- Registration happens before `main`, when the program starts.
- Names are unique per base class. If a name is already registered, the
  later registration is ignored.
- Abstract classes are never registered, so they can use
  `SelfRegisteringClass` safely (for instance, intermediate classes of a
  hierarchy), but they cannot be created.
- The objects are created with `new`. `make` returns ownership to the caller;
  prefer `make_unique` or `make_shared`.

## Plugins

A class can also live in a dynamic library that is loaded when it is needed.
From [examples/self-registering-plugin](../examples/self-registering-plugin):

```cpp
// circle.cc, compiled as a shared library
class Circle : public pattern::SelfRegisteringClass<Circle, Shape>, public Shape { ... };
PATTERN_EXPORT_REGISTERED;

// main.cc
auto circle = pattern::SelfRegisteringFactory<Shape>::make_unique("circle", "self-registering-plugin-circle");
```

- `PATTERN_EXPORT_REGISTERED;` (once per library) exports the entry points
  the factory needs to find and merge the registrations of the library.
- The second argument of `make_unique` (also `make` and `make_shared`) is the
  library name, or a list of names. It is only used if the type is not
  already registered in the executable.
- Libraries are searched on the paths added with
  `SelfRegisteringFactory<Base>::add_library_path(path)`, or in the current
  directory (`./`) by default.
- Define `PATTERN_SHOW_SELF_REGISTERING` before including the library to
  print on `std::cerr` what is being registered and loaded.

### Building plugins with CMake

The library provides CMake functions in [Patterns.cmake](../Patterns.cmake)
so a plugin is built without writing any library-specific CMake code. From
[CMakeLists.txt](../CMakeLists.txt):

```cmake
list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR})
include(Paths)
include(Patterns)

add_plugin(self-registering-plugin-circle examples/self-registering-plugin/circle.cc)
add_executable(self-registering-plugin examples/self-registering-plugin/main.cc)
```

`add_plugin(target sources...)` creates a shared library from the sources.
That one line is all that is needed:

- **Output next to the executable:** [Paths.cmake](../Paths.cmake) sends
  executables and libraries to the same `bin` directory. Since the factory
  looks in `./` by default, the executable finds the plugin when run from
  there, with no copying or extra configuration.
- **Name matches the target:** the factory adds the platform prefix and
  extension itself (`libself-registering-plugin-circle.so` on Linux,
  `libself-registering-plugin-circle.dylib` on macOS,
  `self-registering-plugin-circle.dll` on Windows). `add_plugin` removes the
  `lib` prefix on MinGW so the file name matches that convention. Pass
  the plain target name to `make_unique`, as in the example above.
- **No link dependency:** the executable is not linked against the plugin,
  which is loaded at run time when its type is requested. The plugin and the
  executable only need to agree on the base class (`Shape`).

To keep the plugins of a base type in their own subdirectory of the output
directory, use `add_plugin_on_folder(target folder sources...)`:

```cmake
add_plugin_on_folder(circle shape examples/pimpl-reflect-plugin/circle.cc)
```

This builds `circle` into `bin/shape/` instead of `bin/`. In that case,
register the folder with `SelfRegisteringFactory<Base>::add_library_path`.

Build and run as usual; no manual step is needed to produce or install the
plugin:

```sh
cmake -S . -B build && cmake --build build
cd bin && ./self-registering-plugin
```

## Combining with other patterns

When the base class is also [reflectable](reflect.md) and held by a
[pimpl](pimpl.md), registration comes with `Reflectable<Self, Base>` and the
wrapper finds the plugins by itself, from a name or from XML. See
[Pimpl with reflection and self-registration](pimpl-reflect.md).

