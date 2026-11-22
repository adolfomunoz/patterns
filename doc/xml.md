# XML

Reflectable classes, basic values, collections and reflectable pimpls can be
**saved to and loaded from XML**. The C++ classes expose their attributes once
through reflection; `xml` and the loading functions use the same description
for both directions. The examples are
[examples/xml-reflect-inheritance](../examples/xml-reflect-inheritance) and
[examples/xml-variables](../examples/xml-variables).

XML files can also be supplied as [command-line](commandline.md) arguments,
with command-line variables used to configure them.

## Using XML

From [main.cc](../examples/xml-reflect-inheritance/main.cc):

```cpp
Rectangle rect(2.0f, 2.0f);
std::cout << pattern::xml(rect);

pattern::load_xml(rect,
    "<rectangle>\
        <float name=\"height\" value=\"1.0\"/>\
        <float name=\"x\" value=\"3.0\"/>\
     </rectangle>");
```

The generated XML contains both the attributes of `Rectangle` and those of
its reflectable base:

```xml
<rectangle>
    <float name="width" value="2" />
    <float name="height" value="2" />
    <float name="x" value="0" />
    <float name="y" value="0" />
</rectangle>
```

| Function | Purpose |
|---|---|
| `pattern::xml(x)` | returns the XML text of a basic value, collection or reflectable object |
| `pattern::xml(x, flags)` | returns XML using the requested output format |
| `pattern::load_xml(x, text [, variables])` | loads `x` from XML text, optionally replacing variables first |
| `pattern::load_xml_file(x, path [, variables])` | loads from a file; returns `false` if it cannot be opened |
| `pattern::make_from_xml<T>(text [, variables])` | builds a default-constructed `T` and loads it from XML text |

There is no separate serialization method to write in each class: its
`reflect()`, `reflect_names()` and `type_name()` description is used for both
saving and loading. See [Writing a reflectable class](reflect.md#writing-a-reflectable-class).

### Loading values and collections

A basic value can be written as a child value node or as an attribute, and its
value can be stored in the `value` attribute or in the node content:

```cpp
int i = 0;
pattern::load_xml(i, "<integer value=\"3\"/>");

std::list<float> values;
pattern::load_xml(values,
    "<float value=\"-1.0\"/>"
    "<float value=\"-2.0\"/>"
    "<float value=\"-3.0\"/>");
```

Collections have no collection tag: each element is written consecutively.
Loading a collection clears it and then fills it from all matching elements.

### Loading and saving reflectable inheritance

In [rectangle.h](../examples/xml-reflect-inheritance/rectangle.h), `Rectangle`
is declared as `Reflectable<Rectangle, Shape>`. The attributes from the
reflectable `Shape` base participate in XML together with the attributes of
the derived class:

```cpp
class Rectangle : public pattern::Reflectable<Rectangle, Shape> {
    float width, height;
public:
    auto reflect() { return std::tie(width, height); }
    auto reflect_names() const { return std::tuple("width", "height"); }
    static const char* type_name() { return "rectangle"; }
};
```

Attributes can be supplied as named child nodes or as attributes of the
reflectable object's node. Their order does not matter:

```xml
<rectangle height="1.0" x="3.0"/>
```

An attribute omitted from the XML keeps its current value. This makes a file
usable as a partial update: loading the `Rectangle` above changes `height` and
`x`, while keeping `width` and `y` unchanged. A derived class with no own
attributes still loads the inherited attributes; `Point` in the same example
uses this form.

## Output format

By default, stream-convertible reflected values are child nodes with a `value`
attribute. Alternative formats can be selected with flags:

```cpp
std::cout << pattern::xml(rect, pattern::xml_reflect_attributes_from_stream);
std::cout << pattern::xml(rect, pattern::xml_tag_as_derived);
std::cout << pattern::xml(rect, pattern::xml_value_as_content);
```

| Flag | Effect |
|---|---|
| `pattern::xml_reflect_attributes_from_stream` | writes stream-convertible reflected fields as attributes of the containing node |
| `pattern::xml_tag_as_derived` | for a reflectable pimpl, uses the derived type as the tag instead of the base tag and a `type` attribute |
| `pattern::xml_value_as_content` | writes a basic value as node content instead of a `value` attribute |

For example, with `xml_reflect_attributes_from_stream` the `Rectangle` becomes:

```xml
<rectangle width="2" height="2" x="0" y="0" />
```

## Reflectable pimpls and XML variables

[examples/xml-variables/main.cc](../examples/xml-variables/main.cc) loads a
`std::list<Shape>`. `Shape` is a `Pimpl` over a self-registering reflectable
base, so its XML can select the concrete implementation with `type`:

```xml
<shape type="circle" radius="1"/>
<shape type="rectangle" width="4" height="5"/>
```

Self-registering implementations can therefore be created from XML without a
selection chain in `main.cc`. Saving uses the base tag with a `type` attribute
by default. With `xml_tag_as_derived`, the same objects are written with the
derived tags, such as `<circle radius="1"/>`.

XML also supports text preprocessing variables. The third `load_xml` argument
is a list of `pattern::Default` name/value pairs:

```cpp
pattern::load_xml(shapes, xml_shapes,
    {{"finalshape", replacement_1}});
```

A file or XML string can declare a default, including a complete XML node:

```xml
<default name="finalshape">
    <shape type="circle" radius="3"/>
</default>

<shape type="circle" radius="1"/>
<shape type="circle" radius="2"/>
<$finalshape/>
```

Without an external replacement, `$finalshape` inserts the `<shape>` declared
by the default. `replacement_1` in the example replaces that entire node with a
rectangle of width 2 and height 3; a later load replaces it with a 4 by 5
rectangle.

- **Defaults:** `<default name="name">...</default>` defines a value when no
  external value has the same name. The declaration is removed before loading.
- **External values:** `Default` values passed to `load_xml` override defaults
  declared in the XML.
- **Scalars:** a default can use `value="..."`, and `$name` can occur anywhere
  in the remaining XML, for example `radius="$radius"`.
- **Whole nodes:** `<$name/>` is useful for inserting a complete XML fragment.
  Such fragments are inserted as raw XML, not escaped text.
- **Collection replacement:** each load clears the collection. The first two
  circles in the example are recreated and the final shape is reconstructed
  from the selected replacement.
- **Unknown variables:** text for a variable without either a declaration or an
  external value remains unchanged.

The preprocessing pass is also available directly as
`pattern::replace_variables_in_xml(text, variables)`.

The same variables can be supplied on the command line as `--name=value`; see
[Command line: Loading XML files](commandline.md#loading-xml-files) and
[Command line: Variables](commandline.md#variables).

## Initialization after loading

If the loaded type defines `void init()`, `load_xml`, `load_xml_file` and
`make_from_xml` call it after the complete object has been loaded. This is
useful for rebuilding cached values from reflected attributes. The method is
detected automatically; it is not added to `reflect()`.

## Semantics

- **No reset for reflectable objects:** fields omitted from the XML keep their
  previous value. Collections are the exception and are cleared before load.
- **Type selection:** a reflectable pimpl accepts a base tag with `type` or the
  derived tag directly. An unknown type leaves the wrapper unchanged.
- **Required constructors:** `make_from_xml` requires a default-constructible
  type; collections also need default-constructible elements.
- **Preprocessing:** variable replacement happens before the XML is parsed, so
  defaults and replacement fragments must produce valid XML in their final
  position.
- **Saving:** serialization emits the current object state; variable defaults
  and declarations are not preserved in the generated XML.

See [Reflection](reflect.md) for writing the class descriptions and
[Command line](commandline.md) for loading XML configuration files from program
arguments.
