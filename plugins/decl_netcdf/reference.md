# Decl'NetCDF configuration reference {#Decl_NetCDF_reference}

This page documents the configuration of the \ref Decl_NetCDF_plugin "Decl'NetCDF plugin", one section per key, named after its path as in the
\ref Specification_tree_ref "specification tree reference".
\ref Decl_NetCDF_examples "Decl'NetCDF examples" shows how to combine them.

## `@decl_netcdf` {#decl_netcdf_config}

Either a single \ref decl_netcdf_ncfile or a sequence of them.
Use it, as the value of `plugins.decl_netcdf` (see \ref spectree_pdi_plugins_plugin_name), to describe the files to access.

**Example:**

\snippet{trimleft} decl_netcdf/docs/reference.yml decl_netcdf_config


**Example:**

\snippet{trimleft} decl_netcdf/docs/reference.yml decl_netcdf_config_sequence


### `@decl_netcdf.logging` {#decl_netcdf_logging}

*Optional.*
A \ref spectree_logging.
Use `logging` to configure the logger of the plugin, see \ref spectree_pdi_plugins_plugin_name_logging.
It is only taken into account when the value of `@decl_netcdf` is a single \ref decl_netcdf_ncfile, not a sequence.

**Example:**

\snippet{trimleft} decl_netcdf/docs/reference.yml decl_netcdf_logging


## `@ncfile` {#decl_netcdf_ncfile}

A *file description*: a mapping with the keys below, that specifies the I/O to execute in one file.
Keys not listed here are ignored.
You can use it in \ref decl_netcdf_config.

**Example:**

\snippet{trimleft} decl_netcdf/docs/reference.yml decl_netcdf_ncfile


### `@ncfile.file` {#decl_netcdf_ncfile_file}

*Required.*
A \ref spectree_expression.
Use `file` to give the path of the file to read or write.

**Example:**

\snippet{trimleft} decl_netcdf/docs/reference.yml decl_netcdf_ncfile_file


### `@ncfile.communicator` {#decl_netcdf_ncfile_communicator}

*Optional.*
A \ref spectree_expression referencing a valid MPI communicator.
Use `communicator` to enable parallel NetCDF I/O on this communicator.
All processes must use the same communicator and file name.

\warning This feature is not tested yet.

**Example:**

\snippet{trimleft} decl_netcdf/docs/reference_parallel.yml decl_netcdf_ncfile_communicator


### `@ncfile.on_event` {#decl_netcdf_ncfile_on_event}

*Optional.*
Either a single scalar or a sequence of them.
Use `on_event` to name the events on which the plugin reads data from, or writes data to, the file.
If it is not specified, each data is read or written when it is shared.

**Example:**

\snippet{trimleft} decl_netcdf/docs/reference.yml decl_netcdf_ncfile_on_event


### `@ncfile.when` {#decl_netcdf_ncfile_when}

*Optional.*
A \ref spectree_expression, always true by default.
Use `when` to set the condition under which the plugin accesses the file.

**Example:**

\snippet{trimleft} decl_netcdf/docs/reference.yml decl_netcdf_ncfile_when


### `@ncfile.deflate` {#decl_netcdf_ncfile_deflate}

*Optional.*
An integer-valued \ref spectree_expression, from 0 to 9, 0 by default.
Use `deflate` to set the compression level of all variables of the file.
It can be overridden for each variable, see \ref decl_netcdf_ncfile_variables_variable_path_deflate.

**Example:**

\snippet{trimleft} decl_netcdf/docs/reference.yml decl_netcdf_ncfile_deflate


### `@ncfile.groups.<group_path>` {#decl_netcdf_ncfile_groups_group_path}

A mapping.
Use `groups.<group_path>` to describe the group at path `<group_path>`, where `/` separates nested groups (`group1/group2`).
A group without attributes can be omitted: Decl'NetCDF creates the groups of every variable anyway.

**Example:**

\snippet{trimleft} decl_netcdf/docs/reference.yml decl_netcdf_ncfile_groups_group_path


### `@ncfile.groups.<group_path>.attributes` {#decl_netcdf_ncfile_groups_group_path_attributes}

*Optional.*
A \ref decl_netcdf_ncattributes.
Use `attributes` to read or write the attributes of the group.

**Example:**

\snippet{trimleft} decl_netcdf/docs/reference.yml decl_netcdf_ncfile_groups_group_path_attributes


### `@ncfile.variables.<variable_path>` {#decl_netcdf_ncfile_variables_variable_path}

A mapping.
Use `variables.<variable_path>` to describe the variable at path `<variable_path>`.
A variable in a group has its path prefixed by that of its group: `group_name/variable_name`.
It is mainly used to name the dimensions of the variable, and to read or write its attributes.
A variable that is not described here has the name and type of the descriptor and no attributes, and the plugin names its dimensions
arbitrarily.

**Example:**

\snippet{trimleft} decl_netcdf/docs/reference.yml decl_netcdf_ncfile_variables_variable_path


### `@ncfile.variables.<variable_path>.type` {#decl_netcdf_ncfile_variables_variable_path_type}

*Optional.*
A \ref spectree_datatype_type, whose other keys (`subtype`, `size`...) are given next to it, in \ref decl_netcdf_ncfile_variables_variable_path.
Use `type` to set the type of the variable.
It is required if the type of the variable differs from that of the descriptor.
It also makes it possible to define an `UNLIMITED` dimension, by setting its size to `0`.

**Example:**

\snippet{trimleft} decl_netcdf/docs/reference.yml decl_netcdf_ncfile_variables_variable_path_type


### `@ncfile.variables.<variable_path>.dimensions` {#decl_netcdf_ncfile_variables_variable_path_dimensions}

*Optional.*
A sequence of scalars.
Use `dimensions` to name the dimensions of the variable.
A value that is not a sequence is ignored.

**Example:**

\snippet{trimleft} decl_netcdf/docs/reference.yml decl_netcdf_ncfile_variables_variable_path_dimensions


### `@ncfile.variables.<variable_path>.attributes` {#decl_netcdf_ncfile_variables_variable_path_attributes}

*Optional.*
A \ref decl_netcdf_ncattributes.
Use `attributes` to read or write the attributes of the variable.

**Example:**

\snippet{trimleft} decl_netcdf/docs/reference.yml decl_netcdf_ncfile_variables_variable_path_attributes


### `@ncfile.variables.<variable_path>.deflate` {#decl_netcdf_ncfile_variables_variable_path_deflate}

*Optional.*
An integer-valued \ref spectree_expression, from 0 to 9.
Use `deflate` to set the compression level of the variable, using the native NetCDF compression, with the shuffle filter.
A scalar variable is never compressed.
It defaults to \ref decl_netcdf_ncfile_deflate, and `0` means no compression.

**Example:**

\snippet{trimleft} decl_netcdf/docs/reference.yml decl_netcdf_ncfile_variables_variable_path_deflate


### `@ncfile.variables.<variable_path>.chunking` {#decl_netcdf_ncfile_variables_variable_path_chunking}

*Optional.*
Either a single integer-valued \ref spectree_expression or a sequence of them.
Use `chunking` to set the size of the chunks of the variable; it is only taken into account when the variable is compressed.
It must have as many dimensions as the variable.
If it is not specified, NetCDF chooses the chunking automatically.

**Example:**

\snippet{trimleft} decl_netcdf/docs/reference.yml decl_netcdf_ncfile_variables_variable_path_chunking


### `@ncfile.write.<data_name>` {#decl_netcdf_ncfile_write_data_name}

A \ref decl_netcdf_ncio.
Use `write.<data_name>` to write the descriptor named `<data_name>`.
Its datatype must be dense, without `subsize` or `start`: Decl'NetCDF writes its whole buffer.

\warning To write a record datatype, its `+decl_netcdf.type` attribute must be set to the name of the NetCDF compound type.

**Example:**

\snippet{trimleft} decl_netcdf/docs/reference.yml decl_netcdf_ncfile_write_data_name


To write a record, its type needs the `+decl_netcdf.type` attribute:

\snippet{trimleft} decl_netcdf/docs/reference.yml decl_netcdf_ncfile_write_data_name_compound


### `@ncfile.write[*]` {#decl_netcdf_ncfile_write_item}

A scalar.
Use each element of `write` to write the descriptor it names to the file.
`write` can also be a single name instead of a sequence.
The NetCDF variable has the name of the descriptor, unless it is described in \ref decl_netcdf_ncfile_variables_variable_path.

**Example:**

\snippet{trimleft} decl_netcdf/docs/reference.yml decl_netcdf_ncfile_write_item


### `@ncfile.read.<data_name>` {#decl_netcdf_ncfile_read_data_name}

A \ref decl_netcdf_ncio.
Use `read.<data_name>` to read into the descriptor named `<data_name>`.
Its datatype must be dense, as for \ref decl_netcdf_ncfile_write_data_name.

**Example:**

\snippet{trimleft} decl_netcdf/docs/reference.yml decl_netcdf_ncfile_read_data_name


### `@ncfile.read[*]` {#decl_netcdf_ncfile_read_item}

A scalar.
Use each element of `read` to read the descriptor it names from the file.
`read` can also be a single name instead of a sequence.
The NetCDF variable must have the name of the descriptor and the same type, and no attributes are read.

**Example:**

\snippet{trimleft} decl_netcdf/docs/reference.yml decl_netcdf_ncfile_read_item


## `@ncio` {#decl_netcdf_ncio}

An *I/O description*: a mapping with the keys below, that describes one write or one read of a descriptor.
A few keys only apply to reads, as they state.
You can use it in \ref decl_netcdf_ncfile_write_data_name and \ref decl_netcdf_ncfile_read_data_name.

**Example:**

\snippet{trimleft} decl_netcdf/docs/reference.yml decl_netcdf_ncio


### `@ncio.variable` {#decl_netcdf_ncio_variable}

*Optional.*
A \ref spectree_expression, the name of the descriptor by default.
Use `variable` to give the path of the variable to access, which may be described in \ref decl_netcdf_ncfile_variables_variable_path.

**Example:**

\snippet{trimleft} decl_netcdf/docs/reference.yml decl_netcdf_ncio_variable


### `@ncio.when` {#decl_netcdf_ncio_when}

*Optional.*
A \ref spectree_expression.
Use `when` to set the condition under which the plugin accesses this descriptor.

**Example:**

\snippet{trimleft} decl_netcdf/docs/reference.yml decl_netcdf_ncio_when


### `@ncio.variable_selection` {#decl_netcdf_ncio_variable_selection}

*Optional.*
A mapping with the keys below.
Use `variable_selection` to access only a part (hyperslab) of the NetCDF variable.

**Example:**

\snippet{trimleft} decl_netcdf/docs/reference.yml decl_netcdf_ncio_variable_selection


### `@ncio.variable_selection.start` {#decl_netcdf_ncio_variable_selection_start}

*Optional.*
A sequence of integer-valued \ref spectree_expression "$-expressions".
Use `start` to set the start index of the selection in each dimension.

**Example:**

\snippet{trimleft} decl_netcdf/docs/reference.yml decl_netcdf_ncio_variable_selection_start


### `@ncio.variable_selection.subsize` {#decl_netcdf_ncio_variable_selection_subsize}

*Optional.*
A sequence of integer-valued \ref spectree_expression "$-expressions".
Use `subsize` to set the number of elements (count) of the selection in each dimension.

**Example:**

\snippet{trimleft} decl_netcdf/docs/reference.yml decl_netcdf_ncio_variable_selection_subsize


### `@ncio.size_of` {#decl_netcdf_ncio_size_of}


*Optional, reads only.*
A \ref spectree_expression.
Use `size_of` to read the size of the variable at this path into the descriptor, in place of its content.
If \ref decl_netcdf_ncio_variable is given too, the size of that variable is read instead, and the value of `size_of` is ignored.

**Example:**

\snippet{trimleft} decl_netcdf/docs/reference.yml decl_netcdf_ncio_size_of


## `@ncattributes` {#decl_netcdf_ncattributes}

A mapping: the attributes of a group or of a variable.
Attributes are read and written when the file is opened.
You can use it in \ref decl_netcdf_ncfile_groups_group_path_attributes and \ref decl_netcdf_ncfile_variables_variable_path_attributes.

**Example:**

\snippet{trimleft} decl_netcdf/docs/reference.yml decl_netcdf_ncattributes


### `@ncattributes.<attribute_name>` {#decl_netcdf_ncattributes_attribute_name}

A \ref spectree_expression.
Use `<attribute_name>` to write the attribute named `<attribute_name>` with this value, or to read it into the data it references.

**Example:**

\snippet{trimleft} decl_netcdf/docs/reference.yml decl_netcdf_ncattributes_attribute_name
