# The Decl'NetCDF plugin {#Decl_NetCDF_plugin}

The Decl'NetCDF plugin was created to read from and write to NetCDF files in a declarative way.

Decl'NetCDF plugin allows you to:
1. Read/write data as NetCDF variables from/to NetCDF file.
2. Define groups in the NetCDF file. Groups can be nested.
3. Read/write variables and groups attributes. **Automatically read and written when openning the file.**
4. Execute input/output operations on event.
5. Execute input/output operations on data share (when the event is not defined).
6. Define `UNLIMITED` dimension and dimensions names.
7. Read/write only part of NetCDF variable by hyperslab definition.
8. Read/write in parallel mode using MPI.

## `@decl_netcdf` {#decl_netcdf_config}

Either a single \ref decl_netcdf_ncfile or a sequence of them.
Use it, as the value of `plugins.decl_netcdf` (see \ref spectree_pdi_plugins_plugin_name), to describe the files to access.

**Example:**

```yaml
plugins:
  decl_netcdf:            # <- a single file description...
    file: result.nc
    write: [field]
```


**Example:**

```yaml
plugins:
  decl_netcdf:            # <- ... or a sequence of them, one per file and direction
    - file: result.nc
      write: [field]
    - file: restart.nc
      read: [field]
```


### `@decl_netcdf.logging` {#decl_netcdf_logging}

*Optional.*
A \ref spectree_logging.
Use `logging` to configure the logger of the plugin, see \ref spectree_pdi_plugins_plugin_name_logging.
It is only taken into account when the value of `@decl_netcdf` is a single \ref decl_netcdf_ncfile, not a sequence.

**Example:**

```yaml
logging: debug            # <- only in the single file description form
file: result.nc
write: [field]
```


## `@ncfile` {#decl_netcdf_ncfile}

A *file description*: a mapping with the keys below, that specifies the I/O to execute in one file.
Keys not listed here are ignored.
You can use it in \ref decl_netcdf_config.

**Example:**

```yaml
plugins:
  decl_netcdf:
    file: checkpoint_${iter}.nc   # <- this mapping is a file description:
    on_event: checkpoint          #    which file, when it is accessed,
    write: [field, iter]          #    and what is written to it
```


### `@ncfile.file` {#decl_netcdf_ncfile_file}

*Required.*
A \ref spectree_expression.
Use `file` to give the path of the file to read or write.

**Example:**

```yaml
file: output_${iter}.nc   # <- evaluated at each access
write: [field]
```


### `@ncfile.communicator` {#decl_netcdf_ncfile_communicator}

*Optional.*
A \ref spectree_expression referencing a valid MPI communicator.
Use `communicator` to enable parallel NetCDF I/O on this communicator.
All processes must use the same communicator and file name.

\warning This feature is not tested yet.

**Example:**

```yaml
file: output.nc
communicator: $MPI_COMM_WORLD   # <- all processes access output.nc, the MPI plugin defines $MPI_COMM_WORLD
write: [field]
```


### `@ncfile.on_event` {#decl_netcdf_ncfile_on_event}

*Optional.*
Either a single scalar or a sequence of them.
Use `on_event` to name the events on which the plugin reads data from, or writes data to, the file.
If it is not specified, each data is read or written when it is shared.

**Example:**

```yaml
file: checkpoint.nc
on_event: [checkpoint, finalization]   # <- the file is written on either event
write: [field, iter]
```


### `@ncfile.when` {#decl_netcdf_ncfile_when}

*Optional.*
A \ref spectree_expression, always true by default.
Use `when` to set the condition under which the plugin accesses the file.

**Example:**

```yaml
file: output.nc
when: $iter % 10 = 0      # <- one access out of ten
write: [field]
```


### `@ncfile.deflate` {#decl_netcdf_ncfile_deflate}

*Optional.*
An integer-valued \ref spectree_expression, from 0 to 9, 0 by default.
Use `deflate` to set the compression level of all variables of the file.
It can be overridden for each variable, see \ref decl_netcdf_ncfile_variables_variable_path_deflate.

**Example:**

```yaml
file: output.nc
deflate: 6                # <- every variable of output.nc is compressed, but those that set their own level
write: [field]
```


### `@ncfile.groups.<group_path>` {#decl_netcdf_ncfile_groups_group_path}

A mapping.
Use `groups.<group_path>` to describe the group at path `<group_path>`, where `/` separates nested groups (`group1/group2`).
A group without attributes can be omitted: Decl'NetCDF creates the groups of every variable anyway.

**Example:**

```yaml
file: output.nc
groups:
  results/fields:         # <- the fields group, in the results group
    attributes:
      step: $iter
write:
  field: {variable: results/fields/field}
```


### `@ncfile.groups.<group_path>.attributes` {#decl_netcdf_ncfile_groups_group_path_attributes}

*Optional.*
A \ref decl_netcdf_ncattributes.
Use `attributes` to read or write the attributes of the group.

**Example:**

```yaml
file: output.nc
groups:
  results:
    attributes:           # <- the attributes of the results group
      step: $iter
write:
  field: {variable: results/field}
```


### `@ncfile.variables.<variable_path>` {#decl_netcdf_ncfile_variables_variable_path}

A mapping.
Use `variables.<variable_path>` to describe the variable at path `<variable_path>`.
A variable in a group has its path prefixed by that of its group: `group_name/variable_name`.
It is mainly used to name the dimensions of the variable, and to read or write its attributes.
A variable that is not described here has the name and type of the descriptor and no attributes, and the plugin names its dimensions
arbitrarily.

**Example:**

```yaml
file: output.nc
variables:
  results/field:          # <- the field variable, in the results group
    dimensions: [height, width]
    attributes:
      units: kelvin
write:
  field: {variable: results/field}
```


### `@ncfile.variables.<variable_path>.type` {#decl_netcdf_ncfile_variables_variable_path_type}

*Optional.*
A \ref spectree_datatype_type, whose other keys (`subtype`, `size`...) are given next to it, in \ref decl_netcdf_ncfile_variables_variable_path.
Use `type` to set the type of the variable.
It is required if the type of the variable differs from that of the descriptor.
It also makes it possible to define an `UNLIMITED` dimension, by setting its size to `0`.

**Example:**

```yaml
file: history.nc
variables:
  field:
    type: array           # <- the variable has one more dimension than the data,
    subtype: double
    size: [0, 64, 64]     #    an UNLIMITED one, with one entry per iteration
write:
  field:
    variable_selection: {start: [$iter, 0, 0], subsize: [1, 64, 64]}
```


### `@ncfile.variables.<variable_path>.dimensions` {#decl_netcdf_ncfile_variables_variable_path_dimensions}

*Optional.*
A sequence of scalars.
Use `dimensions` to name the dimensions of the variable.
A value that is not a sequence is ignored.

**Example:**

```yaml
file: output.nc
variables:
  field:
    dimensions: [height, width]   # <- instead of names chosen by the plugin
write: [field]
```


### `@ncfile.variables.<variable_path>.attributes` {#decl_netcdf_ncfile_variables_variable_path_attributes}

*Optional.*
A \ref decl_netcdf_ncattributes.
Use `attributes` to read or write the attributes of the variable.

**Example:**

```yaml
file: output.nc
variables:
  field:
    attributes:           # <- the attributes of the field variable
      units: kelvin
      step: $iter
write: [field]
```


### `@ncfile.variables.<variable_path>.deflate` {#decl_netcdf_ncfile_variables_variable_path_deflate}

*Optional.*
An integer-valued \ref spectree_expression, from 0 to 9.
Use `deflate` to set the compression level of the variable, using the native NetCDF compression, with the shuffle filter.
A scalar variable is never compressed.
It defaults to \ref decl_netcdf_ncfile_deflate, and `0` means no compression.

**Example:**

```yaml
file: output.nc
variables:
  field:
    deflate: 9            # <- overrides the level of the file
write: [field]
```


### `@ncfile.variables.<variable_path>.chunking` {#decl_netcdf_ncfile_variables_variable_path_chunking}

*Optional.*
Either a single integer-valued \ref spectree_expression or a sequence of them.
Use `chunking` to set the size of the chunks of the variable; it is only taken into account when the variable is compressed.
It must have as many dimensions as the variable.
If it is not specified, NetCDF chooses the chunking automatically.

**Example:**

```yaml
file: output.nc
variables:
  field:
    deflate: 6
    chunking: [16, 64]    # <- chunks of 16x64 elements instead of those NetCDF would choose
write: [field]
```


### `@ncfile.write.<data_name>` {#decl_netcdf_ncfile_write_data_name}

A \ref decl_netcdf_ncio.
Use `write.<data_name>` to write the descriptor named `<data_name>`.

\warning To write a record datatype, its `+decl_netcdf.type` attribute must be set to the name of the NetCDF compound type.

**Example:**

```yaml
file: matrix.nc
variables:
  integer_matrix:
    type: array
    subtype: int
    size: [8, 8]
write:
  int_submatrix:          # <- a 4x4 data written to the bottom-right corner of an 8x8 variable
    variable: integer_matrix
    variable_selection:
      start: [4, 4]
      subsize: [4, 4]
```


To write a record, its type needs the `+decl_netcdf.type` attribute:

```yaml
data:
  particle:
    type: struct
    +decl_netcdf.type: particle_xyz   # the name of the NetCDF compound type
    members:
      - x: double
      - y: double
      - z: double
plugins:
  decl_netcdf:
    file: particles.nc
    write: [particle]
```


### `@ncfile.write[*]` {#decl_netcdf_ncfile_write_item}

A scalar.
Use each element of `write` to write the descriptor it names to the file.
`write` can also be a single name instead of a sequence.
The NetCDF variable has the name of the descriptor, unless it is described in \ref decl_netcdf_ncfile_variables_variable_path.

**Example:**

```yaml
file: output.nc
write: [field, iter]      # <- field and iter, each to the variable of the same name
```


### `@ncfile.read.<data_name>` {#decl_netcdf_ncfile_read_data_name}

A \ref decl_netcdf_ncio.
Use `read.<data_name>` to read into the descriptor named `<data_name>`.

**Example:**

```yaml
file: matrix.nc
read:
  matrix_size:
    size_of: integer_matrix   # <- reads the size of the variable, [8, 8]
  int_submatrix:              # <- reads its bottom-right 4x4 corner
    variable: integer_matrix
    variable_selection:
      start: [4, 4]
      subsize: [4, 4]
```


### `@ncfile.read[*]` {#decl_netcdf_ncfile_read_item}

A scalar.
Use each element of `read` to read the descriptor it names from the file.
`read` can also be a single name instead of a sequence.
The NetCDF variable must have the name of the descriptor and the same type, and no attributes are read.

**Example:**

```yaml
file: restart.nc
read: [field, iter]       # <- field and iter, each from the variable of the same name
```


## `@ncio` {#decl_netcdf_ncio}

An *I/O description*: a mapping with the keys below, that describes one write or one read of a descriptor.
A few keys only apply to reads, as they state.
You can use it in \ref decl_netcdf_ncfile_write_data_name and \ref decl_netcdf_ncfile_read_data_name.

**Example:**

```yaml
file: history.nc
write:
  field:                  # <- this mapping is an I/O description
    variable: field_history
    when: $iter > 0
    variable_selection: {start: [$iter, 0], subsize: [1, 64]}
```


### `@ncio.variable` {#decl_netcdf_ncio_variable}

*Optional.*
A \ref spectree_expression, the name of the descriptor by default.
Use `variable` to give the path of the variable to access, which may be described in \ref decl_netcdf_ncfile_variables_variable_path.

**Example:**

```yaml
variable: results/field   # <- the variable field, in the results group
```


### `@ncio.when` {#decl_netcdf_ncio_when}

*Optional.*
A \ref spectree_expression.
Use `when` to set the condition under which the plugin accesses this descriptor.

**Example:**

```yaml
variable: field
when: $iter > 0           # <- not at the first iteration
```


### `@ncio.variable_selection` {#decl_netcdf_ncio_variable_selection}

*Optional.*
A mapping with the keys below.
Use `variable_selection` to access only a part (hyperslab) of the NetCDF variable.

**Example:**

```yaml
variable: field_history
variable_selection:       # <- the row of this iteration
  start: [$iter, 0]
  subsize: [1, 64]
```


### `@ncio.variable_selection.start` {#decl_netcdf_ncio_variable_selection_start}

*Optional.*
A sequence of integer-valued \ref spectree_expression "$-expressions".
Use `start` to set the start index of the selection in each dimension.

**Example:**

```yaml
variable: field_history
variable_selection:
  start: [$iter, 0]       # <- from the row of this iteration
  subsize: [1, 64]
```


### `@ncio.variable_selection.subsize` {#decl_netcdf_ncio_variable_selection_subsize}

*Optional.*
A sequence of integer-valued \ref spectree_expression "$-expressions".
Use `subsize` to set the number of elements (count) of the selection in each dimension.

**Example:**

```yaml
variable: field_history
variable_selection:
  start: [$iter, 0]
  subsize: [1, 64]        # <- a single row of 64 elements
```


### `@ncio.size_of` {#decl_netcdf_ncio_size_of}


*Optional, reads only.*
A \ref spectree_expression.
Use `size_of` to read the size of the variable at this path into the descriptor, in place of its content.
If \ref decl_netcdf_ncio_variable is given too, the size of that variable is read instead, and the value of `size_of` is ignored.

**Example:**

```yaml
size_of: field            # <- reads the dimensions of the field variable, into an array of integers
```


## `@ncattributes` {#decl_netcdf_ncattributes}

A mapping: the attributes of a group or of a variable.
Attributes are read and written when the file is opened.
You can use it in \ref decl_netcdf_ncfile_groups_group_path_attributes and \ref decl_netcdf_ncfile_variables_variable_path_attributes.

**Example:**

```yaml
file: output.nc
variables:
  field:
    attributes:           # <- this mapping is a set of attributes
      units: kelvin
      scale: 0.5
write: [field]
```


### `@ncattributes.<attribute_name>` {#decl_netcdf_ncattributes_attribute_name}

A \ref spectree_expression.
Use `<attribute_name>` to write the attribute named `<attribute_name>` with this value, or to read it into the data it references.

**Example:**

```yaml
units: kelvin             # <- written as is
step: $iter               # <- written from, or read into, the iter data
```


## Full configuration example {#decl_netcdf_full_config}

```yaml
metadata:
  var_attr: float
  group1_attr: float
  group1_data_attr: float
data:
  int_submatrix_top:
    type: array
    subtype: int
    size: [4, 8]
  int_submatrix_bottom:
    type: array
    subtype: int
    size: [4, 8]
plugins:
  decl_netcdf:
    - file: "example.nc"
      on_event: "write"
      groups:
        group_1:
          attributes:
            some_attr: $group1_attr
        group_1/data:
          attributes:
            some_attr: $group1_data_attr
      variables:
        group_1/data/int_matrix:
          type: array
          subtype: int
          size: [8, 8]
          dimensions: ["height", "width"]
          attributes:
            custom_attr: $var_attr
      write: 
        int_submatrix_top:
          variable: group_1/data/int_matrix
          variable_selection:
            start: [0, 0]
            subsize: [4, 8]
        int_submatrix_bottom:
          variable: group_1/data/int_matrix
          variable_selection:
            start: [4, 0]
            subsize: [4, 8]
    - file: "example.nc"
      on_event: "read"
      groups:
        group_1/data:
          attributes:
            some_attr: $custom_attr
      variables:
        group_1/data/int_matrix:
          type: array
          subtype: int
          size: [8, 8]
          dimensions: ["height", "width"]
          attributes:
            custom_attr: $var_attr
      read: 
        int_submatrix_top:
          variable: group_1/data/int_matrix
          variable_selection:
            start: [0, 0]
            subsize: [4, 8]
        int_submatrix_bottom:
          variable: group_1/data/int_matrix
          variable_selection:
            start: [4, 0]
            subsize: [4, 8]
```
