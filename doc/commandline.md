# Command line

Reflectable classes, including [pimpls over reflectable bases](pimpl-reflect.md),
can be **filled from the command line** with no parsing code. The library uses
the same attribute list that serves for [XML](reflect.md): each attribute
becomes an option named after it. It also prints a help text built from that
list (`--help`) and can read the values from `.xml` files given as arguments.
The example is [examples/commandline](../examples/commandline), which uses the
`Shape`, `Circle` and `Rectangle` classes written as in
[Writing the classes](pimpl-reflect.md#writing-the-classes).

## Using it

From [main.cc](../examples/commandline/main.cc):

```cpp
int integer = pattern::make_from_commandline<int>(argc, argv);
float distance = pattern::make_from_commandline<float>(argc, argv, "distance");
auto rect = pattern::make_from_commandline<Rectangle>(argc, argv);
auto shape = pattern::make_from_commandline<Shape>(argc, argv);
std::cout << pattern::xml(shape);
```

```sh
./commandline --integer=3 --distance=2.5 --width=4 --height=5 --type=circle --radius=2
```

| Function | Purpose |
|---|---|
| `pattern::make_from_commandline<T>(argc, argv, [name])` | builds a `T` (default constructible) and loads it from the command line |
| `pattern::load_commandline(t, argc, argv, [name])` | loads an existing object |
| `pattern::exit_if_commandline_help(argc, argv)` | exits the program (status 0) if `--help` is among the arguments |

Nothing is written for `Rectangle` or `Shape`. The optional `name` is the
option name for the value, as `distance` above. Values that are not on the
command line keep the value of a default-constructed `T`. As with
[`load_xml`](reflect.md#loading-from-xml), `init()` is called after loading if
the class has one.

### Option names

| Type | Options | Example |
|---|---|---|
| basic value (number, `std::string`) | `--<name>=value`. Without `name`, the name of the type (`integer`, `float`, `string`) | `--integer=3`, `--distance=2.5` |
| `bool` | `--<name>` (true) or `--<name>=true/false`. Without `name`, `--bool` | `--verbose`, `--verbose=false` |
| `std::optional<T>` | as `T`; the option sets it | `--distance=2.5` |
| reflectable class | one option per attribute, named with its `reflect_names()`, or prefixed with the type name | `--width=4` or `--rectangle-width=4` |
| collection of basic values | the option followed by the values, up to the next `--option` | `--length 1 2 3` |
| reflectable [pimpl](pimpl-reflect.md) | `--type=` or `--<base>-type=` chooses the class, then its attributes | `--type=circle --radius=2` |

- **Reflectable classes:** the prefixed form (`--rectangle-width`) tells apart
  attributes that have the same name in different classes. Attributes that
  are reflectable themselves use their own attributes, prefixed with the name
  of the attribute (`--<attribute>-<name>`).
- **Pimpls:** `--type=circle` (or `--shape-type=circle`) makes the wrapper
  hold a `Circle`, created through the factory, and the attributes of the
  `Circle` are then read as for any reflectable class: `--radius=2`,
  `--circle-radius=2` or `--shape-radius=2`. If the wrapper already holds an
  object and `--type` is not given (or is the same type), the held object is
  loaded in place. Without a type and with an empty wrapper, nothing is
  created and `Shape` stays empty.
- **Collections of pimpls:** a `std::list<Shape>` loaded from the command
  line gets at most one element, chosen as above. This is a current limit.

Because `Shape` is [self-registering](self-registering.md), any class defined
in the executable (or found as a [plugin](pimpl-reflect.md#plugins)) can be
selected with `--type`, with nothing changed in `main.cc`.

## Help: `--help`

If `--help` is among the arguments, `load_commandline` (and so
`make_from_commandline`) prints the options of the type it loads before
loading it. For the example, `./commandline --help` prints, in addition to
the help of `integer` and `distance`, something like:

```
--width=<float>   or   --rectangle-width=<float>
--height=<float>   or   --rectangle-height=<float>
--type=  or  --shape-type=(circle|rectangle)
   circle
    | --radius=<float>   or   --circle-radius=<float>
   rectangle
    | --width=<float>   or   --rectangle-width=<float>
    | --height=<float>   or   --rectangle-height=<float>
```

For a pimpl, the help lists the registered types (in no particular order)
and, under each one, its own options. This is generated from the class
definitions, so it is never out of date when attributes are added.

- **The program goes on:** printing the help does not stop the program, since
  several `load_commandline` calls may each print their part. Call
  `pattern::exit_if_commandline_help(argc, argv)` after all the loads to
  exit once everything has been printed:

  ```cpp
  auto shape = pattern::make_from_commandline<Shape>(argc, argv);
  pattern::exit_if_commandline_help(argc, argv);
  ```

- **Plugin types:** only registered types are listed, so a type that lives in
  a plugin that has not been loaded yet does not appear.
- **Cycles:** if classes contain each other, the help is cut after a few
  levels of nesting.

## Loading XML files

Every argument that ends in `.xml` is read as a file, and its content is
loaded with `load_xml` **before** the command line options are applied. The
files are used for the type being loaded, so they use the same format as
[XML loading](reflect.md#loading-from-xml). From
[main.xml](../examples/commandline/main.xml):

```xml
<float name="distance" value="106.2"/>
<integer value="13"/>
<rectangle>
    <float name="width" value="25"/>
    <float name="height" value="15"/>
</rectangle>
<shape type="circle" radius="3.14"/>
```

```sh
./commandline main.xml                    # all values from the file
./commandline main.xml --distance=7       # the file, with distance replaced
```

- **One file for everything:** each call of `make_from_commandline` reads
  the same file and takes only what is for its type (the `integer`, the
  `distance`, the `rectangle` and the `shape` above).
- **Several files:** their contents are concatenated, so a configuration can
  be split in parts.
- **Precedence:** the order is the default value, then the XML, then the
  command line. An option always wins over the file.
- **Missing files:** a file that cannot be opened is ignored.

### Variables

A value in the XML can be written as `$name`. It is replaced by the value of
`--name=...` from the command line before the XML is loaded, so a file can be
a template. From [configurable.xml](../examples/commandline/configurable.xml):

```xml
<float name="distance" value="$config"/>
<integer value="$config"/>
```

```sh
./commandline configurable.xml --config=5     # distance = 5 and integer = 5
```

- Every `--name=value` argument defines a variable, and not only the ones
  that the program knows.
- Give a value for each variable used. A variable without one is left as
  text, and loading it as a number fails.
- The same options are still applied afterwards, so `--distance=7` also
  overrides what the file produced.

## Semantics

- **Required constructor:** `T` must be default constructible, as
  `make_from_commandline` creates it and then loads it.
- **Parsing is simple:** options must be written with `=` (`--width=4`,
  not `--width 4`), except for collections. Unknown options are ignored.
- **A class can customize it:** a class that defines its own
  `load_commandline` method is loaded through it instead of the
  attribute-based mechanism. `Pimpl` does that for self-registering bases.
