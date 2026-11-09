# Datatypes {#Spectree_types_ref}

A datatype can appear in many places in the specification tree, including in the configuration of plugins.
This page documents each of its keys, named after the path leading to it from the datatype itself, *e.g.* `@array.size` is the `size` key of a
datatype whose `type` is `array`, wherever that datatype appears.
The conventions for these paths are those of the \ref Specification_tree_ref "Specification tree reference".

## `@datatype` {#spectree_datatype}

A *datatype* represents the memory layout and interpretation of data exposed by the application in the \ref Data_store "data store".

A datatype is either:
* a scalar: the name of a type, a shortcut for a mapping with a single \ref spectree_datatype_type key whose value is that scalar,
* a mapping whose \ref spectree_datatype_type key identifies the type, and whose other keys depend on that type.

The paths of the keys that only some types accept start with `@` followed by the name of the type:
`@array.size` is the `size` key of a datatype whose `type` is `array`.
You can use it in \ref spectree_pdi_types_type_name, \ref spectree_pdi_metadata_data_name, \ref spectree_pdi_data_data_name, \ref spectree_array_subtype,
\ref spectree_pointer_subtype, \ref spectree_record_members_member_name, \ref spectree_struct_members_item_member_name and
\ref spectree_tuple_elements_item, as well as in the configuration of plugins.

**Example:**

\snippet{trimleft} Spectree_types/types.yml spectree_datatype

`iter: int` is a shortcut for `iter: {type: int}`.


### `@datatype.type` {#spectree_datatype_type}

*Required.*
A scalar.
Use `type` to name the type of the datatype.

It can be the name of a user-defined datatype (see \ref spectree_pdi_types_type_name), of a type that a plugin makes available, or one of the following.

|`type`                                     |represents                                                        |other keys                                    |
|:------------------------------------------|:-----------------------------------------------------------------|:---------------------------------------------|
|`array`                                    |a potentially multi-dimensional array                             |see \ref spectree_array                       |
|`pointer`                                  |a memory address where data is actually stored                    |see \ref spectree_pointer                     |
|`record`                                   |a structure whose members are placed explicitly                   |see \ref spectree_record                      |
|`struct`                                   |a C `struct` or C++ `class` using the default C memory layout     |see \ref spectree_struct                      |
|`tuple`                                    |a sequence of unnamed elements                                    |see \ref spectree_tuple                       |
|`byte`                                     |the C++ `std::byte` type                                          |                                              |
|`char`                                     |the C `char` type                                                 |                                              |
|`short`                                    |the C `short` type                                                |                                              |
|`int`                                      |the C `int` type                                                  |                                              |
|`long`                                     |the C `long` type                                                 |                                              |
|`long long`                                |the C `long long` type                                            |                                              |
|`unsigned short`                           |the C `unsigned short` type                                       |                                              |
|`unsigned long`                            |the C `unsigned long` type                                        |                                              |
|`unsigned long long`                       |the C `unsigned long long` type                                   |                                              |
|`float`                                    |the C `float` type                                                |                                              |
|`double`                                   |the C `double` type                                               |                                              |
|`int8`, `int16`, `int32`, `int64`          |the C `int8_t`, `int16_t`, `int32_t`, `int64_t` types             |                                              |
|`uint8`, `uint16`, `uint32`, `uint64`      |the C `uint8_t`, `uint16_t`, `uint32_t`, `uint64_t` types         |                                              |
|`int_least8`, ..., `int_least64`           |the C `int_least8_t`, ..., `int_least64_t` types                  |                                              |
|`uint_least8`, ..., `uint_least64`         |the C `uint_least8_t`, ..., `uint_least64_t` types                |                                              |
|`int_fast8`, ..., `int_fast64`             |the C `int_fast8_t`, ..., `int_fast64_t` types                    |                                              |
|`uint_fast8`, ..., `uint_fast64`           |the C `uint_fast8_t`, ..., `uint_fast64_t` types                  |                                              |
|`intmax`, `uintmax`                        |the C `intmax_t` and `uintmax_t` types                            |                                              |
|`intptr`, `uintptr`                        |the C `intptr_t` and `uintptr_t` types                            |                                              |
|`ptrdiff`                                  |the C `ptrdiff_t` type                                            |                                              |
|`size_t`                                   |the C `size_t` type                                               |                                              |
|`character`                                |the Fortran `character` type                                      |see \ref spectree_fortran                     |
|`integer`                                  |the Fortran `integer` type                                        |see \ref spectree_fortran                     |
|`logical`                                  |the Fortran `logical` type                                        |see \ref spectree_fortran                     |
|`real`                                     |the Fortran `real` type                                           |see \ref spectree_fortran                     |

The C types of the `<stdint.h>` and `<stddef.h>` headers can also be written with their `_t` suffix (`int32_t`, `int_fast8_t`, `ptrdiff_t`...),
except `size_t`, which only exists in that form.

The Fortran types are only available when %PDI is built with Fortran support.

**Example:**

\snippet{trimleft} Spectree_types/types.yml spectree_datatype_type


### `@datatype.+<attribute_name>` {#spectree_datatype_attribute_name}

*Optional.*
Anything (**scalar**, **sequence** or **mapping**).
Use `+<attribute_name>` to give the datatype an attribute named `<attribute_name>`, with this value.

Attributes can be added to any datatype given as a mapping.
They are not interpreted by %PDI, but plugins can read them.

**Example:**

\snippet{trimleft} Spectree_types/types.yml spectree_datatype_attribute_name


## `@fortran` {#spectree_fortran}

A \ref spectree_datatype whose \ref spectree_datatype_type is one of the Fortran intrinsic types: `character`, `integer`, `logical` or `real`.
The Fortran types are only available when %PDI is built with Fortran support.

**Example:**

\snippet{trimleft} Spectree_types/types.yml spectree_fortran


### `@fortran.kind` {#spectree_fortran_kind}

*Optional.*
A literal integer.
Use `kind` to set the Fortran *kind* parameter of the type, *e.g.* `integer(kind=...)`.
It is not a \ref spectree_expression, so a `$` reference in it is not evaluated.
If it is missing, 0, or anything other than an integer, the default kind of the Fortran implementation is used; a negative kind is an error.

**Example:**

\snippet{trimleft} Spectree_types/types.yml spectree_fortran_kind


## `@array` {#spectree_array}

A \ref spectree_datatype whose \ref spectree_datatype_type is `array`, a potentially multi-dimensional array.

**Example:**

\snippet{trimleft} Spectree_types/types.yml spectree_array


### `@array.size` {#spectree_array_size}

*Required.*
Either a single integer-valued \ref spectree_expression, for a one-dimensional array, or a sequence of them.
Use `size` to set the size of the array in each dimension (C order).

**Example:**

\snippet{trimleft} Spectree_types/types.yml spectree_array_size

A single value, `size: 10`, makes a one-dimensional array.


### `@array.subtype` {#spectree_array_subtype}

*Required.*
A \ref spectree_datatype.
Use `subtype` to set the type of the elements of the array.

**Example:**

\snippet{trimleft} Spectree_types/types.yml spectree_array_subtype


### `@array.subsize` {#spectree_array_subsize}

*Optional, deprecated.*
Either a single integer-valued \ref spectree_expression or a sequence of them.
Use `subsize` to set the number of elements to actually use in each dimension (slicing).
If specified, it **must** have the same number of elements as \ref spectree_array_size.
It defaults to the full array size in each dimension.

**Example:**

\snippet{trimleft} Spectree_types/types.yml spectree_array_subsize


### `@array.start` {#spectree_array_start}

*Optional, deprecated.*
Either a single integer-valued \ref spectree_expression or a sequence of them.
Use `start` to set the index of the first element to actually use in each dimension (slicing).
If specified, it **must** have the same number of elements as \ref spectree_array_size, and \ref spectree_array_subsize **must** be specified too.
It defaults to 0, the first element, in each dimension.

**Example:**

\snippet{trimleft} Spectree_types/types.yml spectree_array_start


### `@array.order` {#spectree_array_order}

*Optional, deprecated.*
A scalar.
Use `order` to set the order of the dimensions:

|`order`        |the order                             |
|:--------------|:-------------------------------------|
|`C` or `c`     |row-major, the last dimension varies fastest |
Only the C order is supported, and specifying it is deprecated.

**Example:**

\snippet{trimleft} Spectree_types/types.yml spectree_array_order


## `@pointer` {#spectree_pointer}

A \ref spectree_datatype whose \ref spectree_datatype_type is `pointer`, a memory address where data is actually stored.

**Example:**

\snippet{trimleft} Spectree_types/types.yml spectree_pointer


### `@pointer.subtype` {#spectree_pointer_subtype}

*Required.*
A \ref spectree_datatype.
Use `subtype` to set the type of the referenced elements.

**Example:**

\snippet{trimleft} Spectree_types/types.yml spectree_pointer_subtype


## `@record` {#spectree_record}

A \ref spectree_datatype whose \ref spectree_datatype_type is `record`, a structure whose members are placed explicitly.
It may have no members.

**Example:**

\snippet{trimleft} Spectree_types/types.yml spectree_record


### `@record.buffersize` {#spectree_record_buffersize}

*Required.*
An integer-valued \ref spectree_expression.
Use `buffersize` to set the overall size of the record in bytes, including potential padding.

**Example:**

\snippet{trimleft} Spectree_types/types.yml spectree_record_buffersize


### `@record.members.<member_name>` {#spectree_record_members_member_name}

A \ref spectree_datatype given as a mapping with an additional \ref spectree_record_members_member_name_disp key.
Use `members.<member_name>` to define a member named `<member_name>`, of this type.

**Example:**

\snippet{trimleft} Spectree_types/types.yml spectree_record_members_member_name


### `@record.members.<member_name>.disp` {#spectree_record_members_member_name_disp}

*Required.*
An integer-valued \ref spectree_expression.
Use `disp` to place the member at this offset in bytes from the base address of the record.

**Example:**

\snippet{trimleft} Spectree_types/types.yml spectree_record_members_member_name_disp


## `@struct` {#spectree_struct}

A \ref spectree_datatype whose \ref spectree_datatype_type is `struct`, a C `struct` or C++ `class` using the default C memory layout.

**Example:**

\snippet{trimleft} Spectree_types/types.yml spectree_struct

It matches:

\snippet Spectree_types/particle.h struct


### `@struct.members` {#spectree_struct_members}

*Optional.*
An ordered mapping, *i.e.* a sequence of mappings with a single key each.
Use `members` to define the members of the struct, in order.

**Example:**

\snippet{trimleft} Spectree_types/types.yml spectree_struct_members


### `@struct.members[*].<member_name>` {#spectree_struct_members_item_member_name}

A \ref spectree_datatype.
Use `<member_name>` to define a member named `<member_name>`, of this type.

**Example:**

\snippet{trimleft} Spectree_types/types.yml spectree_struct_members_item_member_name


## `@tuple` {#spectree_tuple}

A \ref spectree_datatype whose \ref spectree_datatype_type is `tuple`, a sequence of unnamed elements.

**Example:**

\snippet{trimleft} Spectree_types/types.yml spectree_tuple


### `@tuple.buffersize` {#spectree_tuple_buffersize}

*Optional.*
An integer-valued \ref spectree_expression.
Use `buffersize` to set the overall size of the tuple in bytes, including potential padding.
It **must** be specified if and only if the elements have a \ref spectree_tuple_elements_item_disp.
If omitted, the size is that of the equivalent C `struct`, including the padding that aligns its end.

**Example:**

\snippet{trimleft} Spectree_types/types.yml spectree_tuple_buffersize


### `@tuple.elements` {#spectree_tuple_elements}

*Required.*
A sequence.
Use `elements` to define the elements of the tuple, in order.

**Example:**

\snippet{trimleft} Spectree_types/types.yml spectree_tuple_elements


### `@tuple.elements[*]` {#spectree_tuple_elements_item}

A \ref spectree_datatype, with an optional additional \ref spectree_tuple_elements_item_disp key when it is given as a mapping.
Use each element of `elements` to define an element of the tuple, of this type.

**Example:**

\snippet{trimleft} Spectree_types/types.yml spectree_tuple_elements_item


### `@tuple.elements[*].disp` {#spectree_tuple_elements_item_disp}

*Optional.*
An integer-valued \ref spectree_expression.
Use `disp` to place the element at this offset in bytes from the base address of the tuple.
Either all elements **must** have a `disp` key, or none of them.
If none has one, the elements are laid out as the members of a C `struct`: each at the first offset after the previous one that matches its
alignment.

**Example:**

\snippet{trimleft} Spectree_types/types.yml spectree_tuple_elements_item_disp


