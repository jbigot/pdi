# The Decl'HDF5 plugin {#Decl_HDF5_plugin}

**WARNING** This documentation is a work in progress and does not reflect the
full Decl'HDF5 potential.

The Decl'HDF5 plugin enables one to read and write data from HDF5 files in a
declarative way.
Decl'HDF5 does not support the full HDF5 feature set but offers a simple
declarative interface to access a large subset of it.

This page is the reference of its configuration, one section per key.
\subpage Decl_HDF5_examples "Decl'HDF5 examples" shows, one approach at a time, how to combine them into whole specification trees.

## `@decl_hdf5` {#decl_hdf5_config}

Either a single \ref decl_hdf5_h5file or a sequence of them.
Use it, as the value of `plugins.decl_hdf5` (see \ref spectree_pdi_plugins_plugin_name), to describe the files to access.

**Example:**

```yaml
plugins:
  decl_hdf5:            # <- a single file description...
    file: result.h5
    write: [field]
```

**Example:**

```yaml
plugins:
  decl_hdf5:            # <- ... or a sequence of them, one per file
    - file: result.h5
      write: [field]
    - file: restart.h5
      read: [field]
```

### `@decl_hdf5.logging` {#decl_hdf5_logging}

*Optional.*
A \ref spectree_logging.
Use `logging` to configure the logger of the plugin, see \ref spectree_pdi_plugins_plugin_name_logging.
It is only taken into account when the value of `@decl_hdf5` is a single \ref decl_hdf5_h5file, not a sequence.

**Example:**

```yaml
logging: debug          # <- only in the single file description form
file: result.h5
write: [field]
```

## `@h5file` {#decl_hdf5_h5file}

A *file description*: a mapping with the keys below, that specifies the reads and writes to execute in one file.
You can use it in \ref decl_hdf5_config.

**Example:**

```yaml
plugins:
  decl_hdf5:
    file: checkpoint_${iter}.h5   # <- this mapping is a file description:
    on_event: checkpoint          #    which file, when it is accessed,
    write: [field, iter]          #    and what is written to it
```

Each time the application emits `checkpoint`, `field` and `iter` are written to a file named after the current iteration.

### `@h5file.file` {#decl_hdf5_h5file_file}


*Required.*
A \ref spectree_expression.
Use `file` to name the file that this description refers to.

**Example:**

```yaml
file: output_${iter}.h5   # <- evaluated at each access: output_0.h5, output_1.h5...
write: [field]
```

### `@h5file.on_event` {#decl_hdf5_h5file_on_event}


*Optional.*
Either a single scalar or a sequence of them.
Use `on_event` to name the events on which the whole file is accessed.
If it is not specified, each data is read or written when it is shared, and the file is opened and closed each time.

**Example:**

```yaml
file: checkpoint.h5
on_event: [checkpoint, finalization]   # <- the whole file is written on either event
write: [field, iter]
```

Without `on_event`, `field` and `iter` would each be written as soon as it is shared, in a separate access to the file.

### `@h5file.when` {#decl_hdf5_h5file_when}

*Optional.*
A \ref spectree_expression.
Use `when` to set a default condition to test before executing the reads and writes of this file description.
This condition can be overridden for each I/O, see \ref decl_hdf5_h5io_when.

**Example:**

```yaml
file: output.h5
when: $iter % 10 = 0       # <- one iteration out of ten
write:
  field: {dataset: field_${iter}}
```

### `@h5file.communicator` {#decl_hdf5_h5file_communicator}

*Optional.*
A \ref spectree_expression referencing an MPI communicator.
Use `communicator` to set the communicator of HDF5 parallel synchronized reads and writes.
It defaults to `MPI_COMM_SELF`, which stands for sequential I/O.
For data-triggered (as opposed to event-triggered) reads and writes, it can be overridden for each I/O, see \ref decl_hdf5_h5io_communicator.
It is an error to specify it when the HDF5 library is not parallel.

**Example:**

```yaml
file: output.h5
communicator: $MPI_COMM_WORLD   # <- all processes access the same file, the MPI plugin defines $MPI_COMM_WORLD
datasets:
  field: {type: array, subtype: double, size: [$nprocs, 64, 64]}
write:
  field:
    dataset_selection: {size: [1, 64, 64], start: [$rank, 0, 0]}
```

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

```yaml
file: history.h5
datasets:
  field_history: {type: array, subtype: double, size: [$nb_iter, 64, 64]}   # <- one 3D dataset for all iterations
  'snapshot_[0-9]+': {type: array, subtype: float, size: [64, 64]}         # <- a regex: every snapshot_<n> dataset
write:
  field:
    - dataset: field_history
      dataset_selection: {size: [1, 64, 64], start: [$iter, 0, 0]}
    - dataset: snapshot_${iter}
```

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

```yaml
file: output_${iter}.h5
collision_policy: error   # <- refuse to overwrite the files of a previous run
write: [field]
```

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

```yaml
file: output.h5
deflate: 6                # <- every dataset created in output.h5 is compressed
write:
  field: {chunking: [16, 64]}
```

### `@h5file.fletcher` {#decl_hdf5_h5file_fletcher}


*Optional.*
An integer-valued \ref spectree_expression interpreted as a boolean (0 is false, any other value is true).
Use `fletcher` to enable the Fletcher32 checksum filter by default for the datasets created in this file.
HDF5 only applies filters to chunked datasets: a dataset created with this filter but without \ref decl_hdf5_h5io_chunking fails to be created.
This can be overridden for each write, see \ref decl_hdf5_h5io_fletcher.
By default, the Fletcher32 checksum filter is disabled.
See https://support.hdfgroup.org/HDF5/doc/RM/RM_H5P.html#Property-SetFletcher32 for more information.

**Example:**

```yaml
file: output.h5
fletcher: 1               # <- every dataset created in output.h5 gets a checksum
write:
  field: {chunking: [16, 64]}
```

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

```yaml
file: output.h5
communicator: $MPI_COMM_WORLD
subfiling:                # <- the full form
  count: 4
  policy: CONTINUE
write: [field]
```

**Example:**

```yaml
file: output.h5
communicator: $MPI_COMM_WORLD
subfiling: 4              # <- the short form, the count only
write: [field]
```

### `@h5file.subfiling.count` {#decl_hdf5_h5file_subfiling_count}


*Optional.*
An integer-valued \ref spectree_expression, 0 by default.
Use `count` to set the total number of subfiles to generate.
0 disables subfiling, and a negative value selects the one-subfile-per-node mode.

**Example:**

```yaml
file: output.h5
communicator: $MPI_COMM_WORLD
subfiling:
  count: -1               # <- one subfile per node
write: [field]
```

### `@h5file.subfiling.policy` {#decl_hdf5_h5file_subfiling_policy}


*Optional.*
A \ref spectree_expression.
Use `policy` to choose what to do if the subfiling requirements are not met:

|`policy`          |when the requirements are not met              |
|:-----------------|:----------------------------------------------|
|`STOP` (default)  |a runtime error is raised                      |
|`CONTINUE`        |a standard HDF5 file is written instead        |

**Example:**

```yaml
file: output.h5
communicator: $MPI_COMM_WORLD
subfiling:
  count: 4
  policy: CONTINUE        # <- without MPI_THREAD_MULTIPLE, write a regular file instead of failing
write: [field]
```

### `@h5file.subfiling.stripe_size` {#decl_hdf5_h5file_subfiling_stripe_size}


*Optional.*
A non-negative integer-valued \ref spectree_expression.
Use `stripe_size` to set the amount of data (in bytes) written to one subfile before rotating to the next.
If 0 (the default), the HDF5 default (typically 32MB) is used.

**Example:**

```yaml
file: output.h5
communicator: $MPI_COMM_WORLD
subfiling:
  count: 4
  stripe_size: 8388608    # <- 8 MiB to a subfile before moving to the next one
write: [field]
```

### `@h5file.write.<data_name>` {#decl_hdf5_h5file_write_data_name}

*Optional.*
`write` is either a mapping whose keys are data names, or a sequence (see \ref decl_hdf5_h5file_write_item), never a mix of both.
Without it, nothing is written to the file.

Either a single \ref decl_hdf5_h5io or a sequence of them.
Use `write.<data_name>` to write the %PDI data named `<data_name>`, once per given write.

**Example:**

```yaml
file: output.h5
write:
  field:                  # <- field is written twice at each share:
    - dataset: field_last           # always to the same dataset,
    - dataset: field_${iter}        # and every 10 iterations to its own one
      when: $iter % 10 = 0
```

### `@h5file.write[*]` {#decl_hdf5_h5file_write_item}


A scalar.
Use each element of `write` to write a %PDI data with the default values of \ref decl_hdf5_h5io, either:
* the name of a %PDI data, to write it to the dataset of the same name (*e.g.* `data`, where `data` is both a %PDI data and the name of the HDF5
  dataset),
* the name of a dataset or group, followed by a `#` and the name of a %PDI data, to write the data to the attribute of the same name of that object
  (*e.g.* `dset#attr`, where `attr` is both a %PDI data and the name of the HDF5 attribute of the `dset` dataset).

**Example:**

```yaml
file: output.h5
write: [field, field#iter]   # <- field to the field dataset, iter to its iter attribute
```

### `@h5file.read.<data_name>` {#decl_hdf5_h5file_read_data_name}

*Optional.*
`read` is either a mapping whose keys are data names, or a sequence (see \ref decl_hdf5_h5file_read_item), never a mix of both.
Without it, nothing is read from the file.

Either a single \ref decl_hdf5_h5io or a sequence of them.
Use `read.<data_name>` to read into the %PDI data named `<data_name>`, once per given read.

**Example:**

```yaml
file: restart.h5
read:
  field:                  # <- field is read from another dataset than its name
    dataset: initial_field
```

### `@h5file.read[*]` {#decl_hdf5_h5file_read_item}

A scalar.
Use each element of `read` to read the %PDI data it names from the dataset of the same name, or from an attribute, as for
\ref decl_hdf5_h5file_write_item.

**Example:**

```yaml
file: restart.h5
read: [field, field#iter]   # <- field from the field dataset, iter from its iter attribute
```

## `@h5io` {#decl_hdf5_h5io}

An *I/O description*: a mapping with the keys below, that describes one read or one write of a %PDI data.
All keys are optional and have default values; a few of them only apply to writes, or only to reads, as they state.
You can use it in \ref decl_hdf5_h5file_write_data_name and \ref decl_hdf5_h5file_read_data_name.

A mapping with an \ref decl_hdf5_h5io_attribute key accesses an attribute instead of a dataset, and can only have a \ref decl_hdf5_h5io_when key
besides it.
For a write, such a mapping must be the whole value of \ref decl_hdf5_h5file_write_data_name, not an element of a sequence.

**Example:**

```yaml
file: output.h5
write:
  field:                  # <- this mapping is an I/O description
    dataset: data/field
    when: $iter > 0
    memory_selection: {size: [62, 62], start: [1, 1]}
```

### `@h5io.dataset` {#decl_hdf5_h5io_dataset}

*Optional.*
A \ref spectree_expression.
Use `dataset` to name the dataset to access in the file.
It defaults to the name of the data.
When a written dataset does not exist in the file and its name matches no \ref decl_hdf5_h5file_datasets_dataset_regex, it is created with the
size of the data (that of its `subsize`, for an array that has one), whatever the \ref decl_hdf5_h5io_memory_selection.

**Example:**

```yaml
dataset: group/field_${iter}   # <- one dataset per iteration, in group/
```

### `@h5io.attribute` {#decl_hdf5_h5io_attribute}

*Optional.*
A \ref spectree_expression.
Use `attribute` to access an attribute instead of a dataset.
Its value is the `/`-separated path of the object that holds the attribute (a dataset or a group), followed by a `#` sign and the name of the
attribute itself (*e.g.* `group/dset#attr`).
Without a `#`, the whole value is the path of the object, and the attribute is named after the data.

**Example:**

```yaml
attribute: field#iteration   # <- the data is written to the iteration attribute of the field dataset
```

Used as the I/O description of `iter`, this stores the iteration with the field it describes.

### `@h5io.when` {#decl_hdf5_h5io_when}

*Optional.*
A \ref spectree_expression.
Use `when` to set a condition to test before executing the I/O.
It defaults to \ref decl_hdf5_h5file_when if present, and to an unconditional I/O otherwise.

**Example:**

```yaml
dataset: field_${iter}
when: $iter > 0           # <- replaces the condition of the file, if any
```

### `@h5io.communicator` {#decl_hdf5_h5io_communicator}

*Optional.*
A \ref spectree_expression referencing an MPI communicator.
Use `communicator` to set the communicator of HDF5 parallel synchronized I/O.
Specifying a communicator at this level is incompatible with event-triggered (as opposed to data-triggered) I/O.
It defaults to \ref decl_hdf5_h5file_communicator if present, and to sequential (`MPI_COMM_SELF`) I/O otherwise.

**Example:**

```yaml
communicator: $MPI_COMM_WORLD   # <- this access is parallel, only in a file without on_event
```

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

```yaml
communicator: $MPI_COMM_WORLD
mpio: INDEPENDENT         # <- each process accesses the file on its own, without synchronizing
```

### `@h5io.memory_selection` {#decl_hdf5_h5io_memory_selection}

*Optional.*
A \ref decl_hdf5_h5selection.
Use `memory_selection` to select the part of the data in memory to write, or to read into.
It defaults to selecting the whole data.

**Example:**

```yaml
memory_selection:         # <- the 62x62 interior of a 64x64 field, without its ghost cells
  size: [62, 62]
  start: [1, 1]
```

### `@h5io.dataset_selection` {#decl_hdf5_h5io_dataset_selection}

*Optional.*
A \ref decl_hdf5_h5selection.
Use `dataset_selection` to select the part of the dataset to write to, or to read from.
For a write, this is only valid if the dataset is explicitly defined in \ref decl_hdf5_h5file_datasets_dataset_regex.

**Example:**

```yaml
dataset: field_history
dataset_selection:        # <- the slice of a 3D dataset, defined in datasets, for this iteration
  size: [1, 64, 64]
  start: [$iter, 0, 0]
```

### `@h5io.attributes.<attribute_name>` {#decl_hdf5_h5io_attributes_attribute_name}

A \ref spectree_expression, evaluated when the dataset is accessed.
Use `attributes.<attribute_name>` to access the attribute named `<attribute_name>` of the dataset along with it, the expression being:
* for a write, the value to write to the attribute named `<attribute_name>` of the dataset,
* for a read, a reference to the data in which to store the attribute named `<attribute_name>` of the dataset.

**Example:**

```yaml
dataset: field
attributes:
  iteration: $iter        # <- the field dataset gets an iteration attribute
```

### `@h5io.collision_policy` {#decl_hdf5_h5io_collision_policy}

*Optional, writes only.*
A scalar, one of the values of \ref decl_hdf5_h5file_collision_policy, which it defaults to.
Use `collision_policy` to choose what to do when the dataset already exists.

**Example:**

```yaml
dataset: field
collision_policy: skip_and_warn   # <- keep the first value written, warn at the next ones
```

### `@h5io.chunking` {#decl_hdf5_h5io_chunking}


*Optional, writes only.*
Either a single integer-valued \ref spectree_expression or a sequence of them.
Use `chunking` to set the size of the chunks of the dataset in a chunked layout.
It must have as many dimensions as the dataset type.
This can be overridden by the `+decl_hdf5.chunking` attribute of the dataset type.
By default, the dataset is not chunked.
See https://support.hdfgroup.org/HDF5/doc/RM/RM_H5P.html#Property-SetChunk for more information.

**Example:**

```yaml
dataset: field
chunking: [16, 64]        # <- stored in chunks of 16x64 elements, which compression needs
deflate: 6
```

### `@h5io.deflate` {#decl_hdf5_h5io_deflate}


*Optional, writes only.*
An integer-valued \ref spectree_expression, from 0 to 9, or `-1` for no filter.
Use `deflate` to set the deflate (GNU gzip) compression level of the dataset created by this write.
HDF5 only applies filters to chunked datasets: a dataset created with this filter but without \ref decl_hdf5_h5io_chunking fails to be created.
This can be overridden by the `+decl_hdf5.deflate` attribute of the dataset type.
It defaults to \ref decl_hdf5_h5file_deflate.

**Example:**

```yaml
dataset: field
chunking: [16, 64]
deflate: 9                # <- overrides the level of the file
```

### `@h5io.fletcher` {#decl_hdf5_h5io_fletcher}


*Optional, writes only.*
An integer-valued \ref spectree_expression interpreted as a boolean (0 is false, any other value is true).
Use `fletcher` to enable the Fletcher32 checksum filter for the dataset created by this write.
HDF5 only applies filters to chunked datasets: a dataset created with this filter but without \ref decl_hdf5_h5io_chunking fails to be created.
This can be overridden by the `+decl_hdf5.fletcher` attribute of the dataset type.
It defaults to \ref decl_hdf5_h5file_fletcher.

**Example:**

```yaml
dataset: field
chunking: [16, 64]
fletcher: 1               # <- this dataset gets a checksum
```

### `@h5io.size_of` {#decl_hdf5_h5io_size_of}

*Optional, reads only.*
A \ref spectree_expression.
Use `size_of` to read the size of the dataset at this path into the data, instead of its content.
The data must be an array of `int64`, with one element per dimension of the dataset: the plugin writes 64-bit integers, whatever its type.
When it is present, any other key is ignored.

**Example:**

```yaml
size_of: field            # <- reads the dimensions of the field dataset
```

Used as the I/O description of an array of integers, this lets the application allocate `field` before reading it.

## `@h5selection` {#decl_hdf5_h5selection}

A *selection*: a mapping with the keys below, that selects a rectangular part of the data in memory or of a dataset.
You can use it in \ref decl_hdf5_h5io_memory_selection and \ref decl_hdf5_h5io_dataset_selection.

**Example:**

```yaml
dataset: field
memory_selection:         # <- this mapping is a selection
  size: [62, 62]
  start: [1, 1]
```

### `@h5selection.size` {#decl_hdf5_h5selection_size}

*Optional.*
Either a single integer-valued \ref spectree_expression or a sequence of them.
Use `size` to set the size of the selection in each dimension.
If it is not specified:
* for a memory selection, it defaults to the size of the whole data in each dimension,
* for a dataset selection, it defaults to the size of the memory selection if their numbers of dimensions match, and to the whole dataset
  otherwise.

**Example:**

```yaml
size: [62, 62]            # <- 62x62 elements
start: [1, 1]
```

### `@h5selection.start` {#decl_hdf5_h5selection_start}

*Optional.*
Either a single integer-valued \ref spectree_expression or a sequence of them.
Use `start` to set the number of elements to skip at the beginning of each dimension.
It defaults to 0 in all dimensions.

**Example:**

```yaml
size: [62, 62]
start: [1, 1]             # <- skip the first row and column
```
