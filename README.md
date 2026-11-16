# patterns
C++17 design patterns that I have found that I implement repeatedly and regularly

## Installation

The library is header-only. Everything is included through
[patterns.h](patterns.h), and the only dependency,
[rapidxml](https://github.com/discord/rapidxml), comes as a git submodule in
`ext/rapidxml`. A C++17 compiler is needed.

### Cloning

Clone it with `--recurse-submodules` so that rapidxml is downloaded too:

```sh
git clone --recurse-submodules https://github.com/adolfomunoz/patterns.git
```

If it was cloned without it, get the submodule afterwards:

```sh
git submodule update --init --recursive
```

Then add the directory of the repository to the include path of your project
and include the library:

```cpp
#include <patterns.h>
```

With CMake, `target_include_directories` is enough:

```cmake
target_include_directories(my_target PRIVATE path/to/patterns)
```

### As a submodule of another project

```sh
git submodule add https://github.com/adolfomunoz/patterns.git ext/patterns
git submodule update --init --recursive
```

Anyone cloning your project then uses `--recurse-submodules` (or the
`git submodule update` command above) to get both patterns and its rapidxml.
Add `ext/patterns` to the include path as shown above.

### With CMake FetchContent

```cmake
include(FetchContent)

FetchContent_Declare(patterns
    GIT_REPOSITORY https://github.com/adolfomunoz/patterns.git
    GIT_TAG        main
    GIT_SUBMODULES_RECURSE ON
    SOURCE_SUBDIR  src   # has no CMakeLists.txt: downloads only, the examples are not built
)
FetchContent_MakeAvailable(patterns)

add_executable(my_target main.cc)
target_include_directories(my_target PRIVATE ${patterns_SOURCE_DIR})
```

`GIT_TAG` can be any branch, tag or commit. Fetching the repository downloads
the rapidxml submodule as well.

### Building the examples

The repository has a `CMakeLists.txt` that builds all the
[examples](examples), one executable each. The executables, and the plugins of
the plugin examples, are written to `bin`:

```sh
cmake -S . -B build
cmake --build build
cd bin
```

## Documentation

Each pattern has its own page, with its use, its advantages and what a class
needs to do to take part in it:

| Pattern | Description | Documentation |
|---|---|---|
| **Pimpl** | A value-type wrapper around a polymorphic object. It is used like a plain value but behaves as the derived class it holds, so no pointers are needed. | [doc/pimpl.md](doc/pimpl.md) |
| **Visitor** | Operations defined outside of a class hierarchy that behave differently for each concrete class, through visitor classes or plain lambdas. | [doc/visitor.md](doc/visitor.md) |
| **Reflectable** | A class lists its attributes once, and generic code can go through them, access them by name, and write and load them as XML. | [doc/reflect.md](doc/reflect.md) |
| **Self-registering** | Classes that register themselves in a factory just by being defined, so they can be created from a name, including from plugins in dynamic libraries. | [doc/self-registering.md](doc/self-registering.md) |

The patterns can be combined:

| Combination | Description | Documentation |
|---|---|---|
| **Pimpl and visitor** | Pimpl wrappers that can be visited as the object they hold. | [doc/visitor-pimpl.md](doc/visitor-pimpl.md) |
| **Pimpl, reflectable and self-registering** | Pimpl wrappers that are written to and loaded from XML as the type they hold, and create it by name, even from a plugin. | [doc/pimpl-reflect.md](doc/pimpl-reflect.md) |
| **Command line** | Reading reflectable classes and pimpls from the command line, `--help`, and loading XML files from the command line. | [doc/commandline.md](doc/commandline.md) |

The documentation is based on the programs in [examples](examples).
