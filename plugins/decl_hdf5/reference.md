# Decl'HDF5 configuration reference {#Decl_HDF5_reference}

This page documents the configuration of the \ref Decl_HDF5_plugin "Decl'HDF5 plugin", one section per key, named after its path as in the
\ref Specification_tree_ref "specification tree reference".
\ref Decl_HDF5_examples "Decl'HDF5 examples" shows how to combine them.

## `@decl_hdf5` {#decl_hdf5_config}

Either a single \ref decl_hdf5_h5file or a sequence of them.
Use it, as the value of `plugins.decl_hdf5` (see \ref spectree_pdi_plugins_plugin_name), to describe the files to access.

**Example:**

\snippet{trimleft} decl_hdf5/docs/reference.yml decl_hdf5_config

**Example:**

\snippet{trimleft} decl_hdf5/docs/reference.yml decl_hdf5_config_sequence

### `@decl_hdf5.logging` {#decl_hdf5_logging}

*Optional.*
A \ref spectree_logging.
Use `logging` to configure the logger of the plugin, see \ref spectree_pdi_plugins_plugin_name_logging.
It is only taken into account when the value of `@decl_hdf5` is a single \ref decl_hdf5_h5file, not a sequence.

**Example:**

\snippet{trimleft} decl_hdf5/docs/reference.yml decl_hdf5_logging

## `@h5file` {#decl_hdf5_h5file}

A *file description*: a mapping with the keys below, that specifies the reads and writes to execute in one file.
You can use it in \ref decl_hdf5_config.

**Example:**

\snippet{trimleft} decl_hdf5/docs/reference.yml decl_hdf5_h5file

Each time the application emits `checkpoint`, `field` and `iter` are written to a file named after the current iteration.

### `@h5file.file` {#decl_hdf5_h5file_file}


*Required.*
A \ref spectree_expression.
Use `file` to name the file that this description refers to.

**Example:**

\snippet{trimleft} decl_hdf5/docs/reference.yml decl_hdf5_h5file_file

### `@h5file.on_event` {#decl_hdf5_h5file_on_event}


*Optional.*
Either a single scalar or a sequence of them.
Use `on_event` to name the events on which the whole file is accessed.
If it is not specified, each data is read or written when it is shared, and the file is opened and closed each time.

**Example:**

\snippet{trimleft} decl_hdf5/docs/reference.yml decl_hdf5_h5file_on_event

Without `on_event`, `field` and `iter` would each be written as soon as it is shared, in a separate access to the file.

### `@h5file.when` {#decl_hdf5_h5file_when}

*Optional.*
A \ref spectree_expression.
Use `when` to set a default condition to test before executing the reads and writes of this file description.
This condition can be overridden for each I/O, see \ref decl_hdf5_h5io_when.

**Example:**

\snippet{trimleft} decl_hdf5/docs/reference.yml decl_hdf5_h5file_when

### `@h5file.communicator` {#decl_hdf5_h5file_communicator}

*Optional.*
A \ref spectree_expression referencing an MPI communicator.
Use `communicator` to set the communicator of HDF5 parallel synchronized reads and writes.
It defaults to `MPI_COMM_SELF`, which stands for sequential I/O.
For data-triggered (as opposed to event-triggered) reads and writes, it can be overridden for each I/O, see \ref decl_hdf5_h5io_communicator.
It is an error to specify it when the HDF5 library is not parallel.

**Example:**

\snippet{trimleft} decl_hdf5/docs/reference_parallel.yml decl_hdf5_h5file_communicator

Each process writes its own `field` to its own slice of a 3D dataset.

### `@h5file.datasets.<dataset_regex>` {#decl_hdf5_h5file_datasets_dataset_regex}


A \ref spectree_datatype.
Use `datasets.<dataset_regex>` to set the type of the datasets whose name matches `<dataset_regex>`, created with this type on first access.
A dataset that is accessed but whose name matches no `<dataset_regex>` is created just big enough to fit the data.

`<dataset_regex>` is a regular expression, using the modified ECMAScript regular expression grammar, so that a single entry can define "generic
keys" for several datasets.
It must match the whole name of the dataset, and a dataset whose name matches several of them is an error.
Unlike in Decl'NetCDF, a size of 0 does not make a dimension unlimited: Decl'HDF5 creates every dataset with a fixed size, which it never extends.

**Example:**

\snippet{trimleft} decl_hdf5/docs/reference.yml decl_hdf5_h5file_datasets_dataset_regex

HDF5 converts `field` from `double` to `float` when it writes the snapshots.

### `@h5file.collision_policy` {#decl_hdf5_h5file_collision_policy}


*Optional.*
A scalar.
Use `collision_policy` to choose what to do when writing to a file or dataset that already exists, among the following values
(case-insensitive):

|`collision_policy`        |what happens to the existing file or dataset                              |
|:-------------------------|:-------------------------------------------------------------------------|
|`skip`                    |nothing, it is left as is                                                 |
|`skip_and_warn`           |nothing, but a warning is logged                                          |
|`error`                   |nothing, but an error is raised                                           |
|`write_into` (default)    |it is written into, possibly overwriting the data it holds                |
|`write_into_and_warn`     |it is written into, possibly overwriting the data it holds, and a warning is logged |
|`replace`                 |it is deleted, and a new one is created                                   |
|`replace_and_warn`        |it is deleted, a new one is created, and a warning is logged              |

This can be overridden for each write, see \ref decl_hdf5_h5io_collision_policy.

**Example:**

\snippet{trimleft} decl_hdf5/docs/reference.yml decl_hdf5_h5file_collision_policy

The policy is applied each time the file is accessed: with a file written at each share, `replace` would only keep the last value.

### `@h5file.deflate` {#decl_hdf5_h5file_deflate}


*Optional.*
An integer-valued \ref spectree_expression, from 0 to 9.
Use `deflate` to set the default deflate (GNU gzip) compression level of the datasets created in this file.
HDF5 only applies filters to chunked datasets: a dataset created with this filter but without \ref decl_hdf5_h5io_chunking fails to be created.
This can be overridden for each write, see \ref decl_hdf5_h5io_deflate.
By default, the deflate filter is disabled.
See https://support.hdfgroup.org/HDF5/doc/RM/RM_H5P.html#Property-SetDeflate for more information.

**Example:**

\snippet{trimleft} decl_hdf5/docs/reference.yml decl_hdf5_h5file_deflate

### `@h5file.fletcher` {#decl_hdf5_h5file_fletcher}


*Optional.*
An integer-valued \ref spectree_expression interpreted as a boolean (0 is false, any other value is true).
Use `fletcher` to enable the Fletcher32 checksum filter by default for the datasets created in this file.
HDF5 only applies filters to chunked datasets: a dataset created with this filter but without \ref decl_hdf5_h5io_chunking fails to be created.
This can be overridden for each write, see \ref decl_hdf5_h5io_fletcher.
By default, the Fletcher32 checksum filter is disabled.
See https://support.hdfgroup.org/HDF5/doc/RM/RM_H5P.html#Property-SetFletcher32 for more information.

**Example:**

\snippet{trimleft} decl_hdf5/docs/reference.yml decl_hdf5_h5file_fletcher

### `@h5file.subfiling` {#decl_hdf5_h5file_subfiling}


*Optional.*
A mapping with the keys below, or a scalar: a shortcut for a mapping whose only key is \ref decl_hdf5_h5file_subfiling_count.
Use `subfiling` to configure the HDF5 Subfiling Virtual File Driver (VFD).
This feature is only available if the underlying HDF5 installation was built with `H5_HAVE_SUBFILING_VFD`, and it only applies to the files
accessed in parallel, with a \ref decl_hdf5_h5file_communicator other than `MPI_COMM_SELF`.

Some notes on using `subfiling` correctly:
* **MPI threading requirement**: MPI must be initialized with `MPI_THREAD_MULTIPLE`.
  Otherwise, the plugin raises a runtime error, unless \ref decl_hdf5_h5file_subfiling_policy is `CONTINUE`, in which case the simulation proceeds
  with standard HDF5 I/O.
* **Stripe size configuration**: the stripe size can be set either with \ref decl_hdf5_h5file_subfiling_stripe_size or with the
  `H5FD_SUBFILING_STRIPE_SIZE` environment variable; `stripe_size` takes precedence when both are set.
* **Custom file locations**: to store the subfiles in a specific directory (*e.g.* a fast scratch burst buffer), set the
  `H5FD_SUBFILING_SUBFILE_PREFIX` environment variable to its absolute path.
  **Note**: you must set `H5FD_SUBFILING_CONFIG_FILE_PREFIX` to the same path, so that the main `.h5` file can locate its subfiles.
* **Naming convention**: HDF5 names the subfiles itself, and their names cannot easily be customized.
  They follow a pattern similar to `[filename].h5.subfile_[contextID]_[subfile_index]_of_[total_subfiles]` (*e.g.*
  `output.h5.subfile_11273556_01_of_10`).

**Example:**

\snippet{trimleft} decl_hdf5/docs/reference_parallel.yml decl_hdf5_h5file_subfiling

**Example:**

\snippet{trimleft} decl_hdf5/docs/reference_parallel.yml decl_hdf5_h5file_subfiling_short

### `@h5file.subfiling.count` {#decl_hdf5_h5file_subfiling_count}


*Optional.*
An integer-valued \ref spectree_expression, 0 by default.
Use `count` to set the total number of subfiles to generate.
0 disables subfiling, and a negative value selects the one-subfile-per-node mode.

**Example:**

\snippet{trimleft} decl_hdf5/docs/reference_parallel.yml decl_hdf5_h5file_subfiling_count

### `@h5file.subfiling.policy` {#decl_hdf5_h5file_subfiling_policy}


*Optional.*
A \ref spectree_expression.
Use `policy` to choose what to do if the subfiling requirements are not met:

|`policy`          |when the requirements are not met              |
|:-----------------|:----------------------------------------------|
|`STOP` (default)  |a runtime error is raised                      |
|`CONTINUE`        |a standard HDF5 file is written instead        |

**Example:**

\snippet{trimleft} decl_hdf5/docs/reference_parallel.yml decl_hdf5_h5file_subfiling_policy

### `@h5file.subfiling.stripe_size` {#decl_hdf5_h5file_subfiling_stripe_size}


*Optional.*
A non-negative integer-valued \ref spectree_expression.
Use `stripe_size` to set the amount of data (in bytes) written to one subfile before rotating to the next.
If 0 (the default), the HDF5 default (typically 32MB) is used.

**Example:**

\snippet{trimleft} decl_hdf5/docs/reference_parallel.yml decl_hdf5_h5file_subfiling_stripe_size

### `@h5file.write.<data_name>` {#decl_hdf5_h5file_write_data_name}

*Optional.*
`write` is either a mapping whose keys are data names, or a sequence (see \ref decl_hdf5_h5file_write_item), never a mix of both.
Without it, nothing is written to the file.

Either a single \ref decl_hdf5_h5io or a sequence of them.
Use `write.<data_name>` to write the %PDI data named `<data_name>`, once per given write.

**Example:**

\snippet{trimleft} decl_hdf5/docs/reference.yml decl_hdf5_h5file_write_data_name

### `@h5file.write[*]` {#decl_hdf5_h5file_write_item}


A scalar.
Use each element of `write` to write a %PDI data with the default values of \ref decl_hdf5_h5io, either:
* the name of a %PDI data, to write it to the dataset of the same name (*e.g.* `data`, where `data` is both a %PDI data and the name of the HDF5
  dataset),
* the name of a dataset or group, followed by a `#` and the name of a %PDI data, to write the data to the attribute of the same name of that object
  (*e.g.* `dset#attr`, where `attr` is both a %PDI data and the name of the HDF5 attribute of the `dset` dataset).

**Example:**

\snippet{trimleft} decl_hdf5/docs/reference.yml decl_hdf5_h5file_write_item

### `@h5file.read.<data_name>` {#decl_hdf5_h5file_read_data_name}

*Optional.*
`read` is either a mapping whose keys are data names, or a sequence (see \ref decl_hdf5_h5file_read_item), never a mix of both.
Without it, nothing is read from the file.

Either a single \ref decl_hdf5_h5io or a sequence of them.
Use `read.<data_name>` to read into the %PDI data named `<data_name>`, once per given read.

**Example:**

\snippet{trimleft} decl_hdf5/docs/reference.yml decl_hdf5_h5file_read_data_name

### `@h5file.read[*]` {#decl_hdf5_h5file_read_item}

A scalar.
Use each element of `read` to read the %PDI data it names from the dataset of the same name, or from an attribute, as for
\ref decl_hdf5_h5file_write_item.

**Example:**

\snippet{trimleft} decl_hdf5/docs/reference.yml decl_hdf5_h5file_read_item

## `@h5io` {#decl_hdf5_h5io}

An *I/O description*: a mapping with the keys below, that describes one read or one write of a %PDI data.
All keys are optional and have default values; a few of them only apply to writes, or only to reads, as they state.
You can use it in \ref decl_hdf5_h5file_write_data_name and \ref decl_hdf5_h5file_read_data_name.

A mapping with an \ref decl_hdf5_h5io_attribute key accesses an attribute instead of a dataset, and can only have a \ref decl_hdf5_h5io_when key
besides it.
For a write, such a mapping must be the whole value of \ref decl_hdf5_h5file_write_data_name, not an element of a sequence.

**Example:**

\snippet{trimleft} decl_hdf5/docs/reference.yml decl_hdf5_h5io

### `@h5io.dataset` {#decl_hdf5_h5io_dataset}

*Optional.*
A \ref spectree_expression.
Use `dataset` to name the dataset to access in the file.
It defaults to the name of the data.
When a written dataset does not exist in the file and its name matches no \ref decl_hdf5_h5file_datasets_dataset_regex, it is created with the
size of the data (that of its `subsize`, for an array that has one), whatever the \ref decl_hdf5_h5io_memory_selection.

**Example:**

\snippet{trimleft} decl_hdf5/docs/reference.yml decl_hdf5_h5io_dataset

### `@h5io.attribute` {#decl_hdf5_h5io_attribute}

*Optional.*
A \ref spectree_expression.
Use `attribute` to access an attribute instead of a dataset.
Its value is the `/`-separated path of the object that holds the attribute (a dataset or a group), followed by a `#` sign and the name of the
attribute itself (*e.g.* `group/dset#attr`).
Without a `#`, the whole value is the path of the object, and the attribute is named after the data.

**Example:**

\snippet{trimleft} decl_hdf5/docs/reference.yml decl_hdf5_h5io_attribute

Used as the I/O description of `iter`, this stores the iteration with the field it describes.

### `@h5io.when` {#decl_hdf5_h5io_when}

*Optional.*
A \ref spectree_expression.
Use `when` to set a condition to test before executing the I/O.
It defaults to \ref decl_hdf5_h5file_when if present, and to an unconditional I/O otherwise.

**Example:**

\snippet{trimleft} decl_hdf5/docs/reference.yml decl_hdf5_h5io_when

### `@h5io.communicator` {#decl_hdf5_h5io_communicator}

*Optional.*
A \ref spectree_expression referencing an MPI communicator.
Use `communicator` to set the communicator of HDF5 parallel synchronized I/O.
Specifying a communicator at this level is incompatible with event-triggered (as opposed to data-triggered) I/O.
It defaults to \ref decl_hdf5_h5file_communicator if present, and to sequential (`MPI_COMM_SELF`) I/O otherwise.

**Example:**

\snippet{trimleft} decl_hdf5/docs/reference_parallel.yml decl_hdf5_h5io_communicator

### `@h5io.mpio` {#decl_hdf5_h5io_mpio}

*Optional.*
A scalar.
Use `mpio` to choose the MPI-IO transfer mode of the operation:

|`mpio`                  |the processes access the file          |
|:-----------------------|:--------------------------------------|
|`COLLECTIVE` (default)  |together, synchronizing their accesses |
|`INDEPENDENT`           |each on its own                        |

It is not a $-expression: it must be written as one of these two values.

**Example:**

\snippet{trimleft} decl_hdf5/docs/reference_parallel.yml decl_hdf5_h5io_mpio

### `@h5io.memory_selection` {#decl_hdf5_h5io_memory_selection}

*Optional.*
A \ref decl_hdf5_h5selection.
Use `memory_selection` to select the part of the data in memory to write, or to read into.
It defaults to selecting the whole data.

**Example:**

\snippet{trimleft} decl_hdf5/docs/reference.yml decl_hdf5_h5io_memory_selection

### `@h5io.dataset_selection` {#decl_hdf5_h5io_dataset_selection}

*Optional.*
A \ref decl_hdf5_h5selection.
Use `dataset_selection` to select the part of the dataset to write to, or to read from.
For a write, this is only valid if the dataset is explicitly defined in \ref decl_hdf5_h5file_datasets_dataset_regex.

**Example:**

\snippet{trimleft} decl_hdf5/docs/reference.yml decl_hdf5_h5io_dataset_selection

### `@h5io.attributes.<attribute_name>` {#decl_hdf5_h5io_attributes_attribute_name}

A \ref spectree_expression, evaluated when the dataset is accessed.
Use `attributes.<attribute_name>` to access the attribute named `<attribute_name>` of the dataset along with it, the expression being:
* for a write, the value to write to the attribute named `<attribute_name>` of the dataset,
* for a read, a reference to the data in which to store the attribute named `<attribute_name>` of the dataset.

**Example:**

\snippet{trimleft} decl_hdf5/docs/reference.yml decl_hdf5_h5io_attributes_attribute_name

### `@h5io.collision_policy` {#decl_hdf5_h5io_collision_policy}

*Optional, writes only.*
A scalar, one of the values of \ref decl_hdf5_h5file_collision_policy, which it defaults to.
Use `collision_policy` to choose what to do when the dataset already exists.

**Example:**

\snippet{trimleft} decl_hdf5/docs/reference.yml decl_hdf5_h5io_collision_policy

### `@h5io.chunking` {#decl_hdf5_h5io_chunking}


*Optional, writes only.*
Either a single integer-valued \ref spectree_expression or a sequence of them.
Use `chunking` to set the size of the chunks of the dataset in a chunked layout.
It must have as many dimensions as the dataset type.
This can be overridden by the `+decl_hdf5.chunking` attribute of the dataset type.
By default, the dataset is not chunked.
See https://support.hdfgroup.org/HDF5/doc/RM/RM_H5P.html#Property-SetChunk for more information.

**Example:**

\snippet{trimleft} decl_hdf5/docs/reference.yml decl_hdf5_h5io_chunking

### `@h5io.deflate` {#decl_hdf5_h5io_deflate}


*Optional, writes only.*
An integer-valued \ref spectree_expression, from 0 to 9, or `-1` for no filter.
Use `deflate` to set the deflate (GNU gzip) compression level of the dataset created by this write.
HDF5 only applies filters to chunked datasets: a dataset created with this filter but without \ref decl_hdf5_h5io_chunking fails to be created.
This can be overridden by the `+decl_hdf5.deflate` attribute of the dataset type.
It defaults to \ref decl_hdf5_h5file_deflate.

**Example:**

\snippet{trimleft} decl_hdf5/docs/reference.yml decl_hdf5_h5io_deflate

### `@h5io.fletcher` {#decl_hdf5_h5io_fletcher}


*Optional, writes only.*
An integer-valued \ref spectree_expression interpreted as a boolean (0 is false, any other value is true).
Use `fletcher` to enable the Fletcher32 checksum filter for the dataset created by this write.
HDF5 only applies filters to chunked datasets: a dataset created with this filter but without \ref decl_hdf5_h5io_chunking fails to be created.
This can be overridden by the `+decl_hdf5.fletcher` attribute of the dataset type.
It defaults to \ref decl_hdf5_h5file_fletcher.

**Example:**

\snippet{trimleft} decl_hdf5/docs/reference.yml decl_hdf5_h5io_fletcher

### `@h5io.size_of` {#decl_hdf5_h5io_size_of}

*Optional, reads only.*
A \ref spectree_expression.
Use `size_of` to read the size of the dataset at this path into the data, instead of its content.
The data must be an array of `int64`, with one element per dimension of the dataset: the plugin writes 64-bit integers, whatever its type.
When it is present, any other key is ignored.

**Example:**

\snippet{trimleft} decl_hdf5/docs/reference.yml decl_hdf5_h5io_size_of

Used as the I/O description of an array of integers, this lets the application allocate `field` before reading it.

## `@h5selection` {#decl_hdf5_h5selection}

A *selection*: a mapping with the keys below, that selects a rectangular part of the data in memory or of a dataset.
You can use it in \ref decl_hdf5_h5io_memory_selection and \ref decl_hdf5_h5io_dataset_selection.

**Example:**

\snippet{trimleft} decl_hdf5/docs/reference.yml decl_hdf5_h5selection

### `@h5selection.size` {#decl_hdf5_h5selection_size}

*Optional.*
Either a single integer-valued \ref spectree_expression or a sequence of them.
Use `size` to set the size of the selection in each dimension.
If it is not specified:
* for a memory selection, it defaults to the size of the whole data in each dimension,
* for a dataset selection, it defaults to the size of the memory selection if their numbers of dimensions match, and to the whole dataset
  otherwise.

**Example:**

\snippet{trimleft} decl_hdf5/docs/reference.yml decl_hdf5_h5selection_size

### `@h5selection.start` {#decl_hdf5_h5selection_start}

*Optional.*
Either a single integer-valued \ref spectree_expression or a sequence of them.
Use `start` to set the number of elements to skip at the beginning of each dimension.
It defaults to 0 in all dimensions.

**Example:**

\snippet{trimleft} decl_hdf5/docs/reference.yml decl_hdf5_h5selection_start
