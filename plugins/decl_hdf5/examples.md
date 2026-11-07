# Decl'HDF5 examples {#Decl_HDF5_examples}

Each example demonstrates one feature of the \ref Decl_HDF5_plugin "Decl'HDF5 plugin" with the `decl_hdf5` part of a specification tree;
the \ref Decl_HDF5_reference "reference" documents each of their keys.
The whole specification trees are in `plugins/decl_hdf5/docs/` in %PDI's sources, where %PDI's tests run them and check the files they write.

## The data {#decl_hdf5_ex_program}

All the examples but the last one configure the same program.
Each MPI process owns a block of 4x6 values, with a layer of ghost cells all around it, that it updates over 3 iterations.
It shares these data with %PDI:

| data     | type                                        | shared                                                  |
|:---------|:--------------------------------------------|:--------------------------------------------------------|
| `rank`   | `int` metadata                              | once, at the start, with the `init` event               |
| `nprocs` | `int` metadata                              | once, at the start, with the `init` event               |
| `size`   | metadata array of 2 `int`                   | once, at the start, with the `init` event: `[4, 6]`, the size of the block without its ghost cells |
| `iter`   | `int` metadata                              | at each iteration, with `field` and the `iteration` event: 0, 1, 2 |
| `field`  | array of `$size[0] + 2` x `$size[1] + 2` `double` | at each iteration, with `iter` and the `iteration` event |

To make the files easy to read, each value of `field` tells where it comes from: 100 times the iteration, plus 10 times the global row, plus
the column, and the ghost cells hold -1.
The examples run on a single process unless they say otherwise.

## Writing data when it is shared {#decl_hdf5_ex_simple}

The simplest configuration names a \ref decl_hdf5_h5file_file and the data to \ref decl_hdf5_h5file_write_data_name "write": each one is
written to the dataset of the same name each time the program shares it.

\snippet{trimleft} decl_hdf5/docs/simple.yml decl_hdf5

`simple.h5` holds a dataset `size`, of 2 integers, and a dataset `field`, of 6x8 values.
`field` is written at each iteration to the same dataset, so the file only keeps the last iteration, ghost cells included.
The rest of the examples address these limits one by one.

## Naming files and datasets after the data, and writing conditionally {#decl_hdf5_ex_names}

\ref decl_hdf5_h5file_file and \ref decl_hdf5_h5io_dataset are \ref spectree_expression "$-expressions": they can name a file per process,
and a dataset per iteration.
\ref decl_hdf5_h5io_when restricts a write to the iterations where a condition holds.
A data to write can be given an \ref decl_hdf5_h5io "I/O description", whose keys all have a default value.

\snippet{trimleft} decl_hdf5/docs/names.yml decl_hdf5

`iter`, `rank` and `size` are metadata: %PDI keeps a copy of their last value, which the $-expressions read.
`iter` is shared with `field`, so the name of the dataset is that of the current iteration.
`output_0.h5` holds the datasets `size`, `field_1` and `field_2`.

## Writing a data several times {#decl_hdf5_ex_several}

A \ref decl_hdf5_h5file_write_item "list of I/O descriptions" writes the data once for each of them, each with its own keys: here, always to
the same dataset, and at the even iterations to a dataset of its own.

\snippet{trimleft} decl_hdf5/docs/several.yml decl_hdf5

`output_0.h5` holds the datasets `field_0`, `field_2` and `field_last`.

## Writing on an event, to groups {#decl_hdf5_ex_event}

Without \ref decl_hdf5_h5file_on_event, the file is opened and closed for each data written.
With it, the whole file description is executed at once when the program emits the event, here `iteration`, which the program emits while it
shares both `iter` and `field`.
A `/` in the name of a \ref decl_hdf5_h5io_dataset "dataset" puts it in a group, which Decl'HDF5 creates.

\snippet{trimleft} decl_hdf5/docs/event.yml decl_hdf5

`output_0.h5` holds a group per iteration, `iter0`, `iter1` and `iter2`, each with a scalar dataset `iter` and a dataset `field`.

## Writing attributes {#decl_hdf5_ex_attributes}

\ref decl_hdf5_h5io_attributes_attribute_name writes attributes along with a dataset, each from a $-expression.
\ref decl_hdf5_h5io_attribute writes a data to an attribute instead of a dataset: here `iter`, to an attribute of the dataset of the current
iteration.
The file is written on the `iteration` event, so that the dataset exists when its attribute is written.

\snippet{trimleft} decl_hdf5/docs/attributes.yml decl_hdf5

Each dataset `field_<n>` of `output_0.h5` has an integer attribute `rank`, 0, and an integer attribute `iteration`, `<n>`.

## Leaving the ghost cells out {#decl_hdf5_ex_memory_selection}

A \ref decl_hdf5_h5io_memory_selection writes only a part of the data: here, the 4x6 interior of the last iteration.
A dataset that is not described in \ref decl_hdf5_h5file_datasets_dataset_regex "datasets" is created with the size of the whole data, so the
dataset is described there with the size of the selection.

\snippet{trimleft} decl_hdf5/docs/memory_selection.yml decl_hdf5

The dataset `field` of `output_0.h5` holds 4x6 values, from 200 to 235.

The datatype of the data can tell the same, with the `subsize` and `start` keys of an \ref spectree_array "array": the data is then only its
interior, wherever it is used, and the dataset created for it has its size.
With `field` declared as follows, in the `data` section of the specification tree:

\snippet{trimleft} decl_hdf5/docs/subsize.yml data

This configuration writes the same file:

\snippet{trimleft} decl_hdf5/docs/subsize.yml decl_hdf5

## Writing every iteration to one dataset {#decl_hdf5_ex_time_series}

A \ref decl_hdf5_h5io_dataset_selection writes the data to a part of a larger dataset: here, each iteration to its own slice of a 3D dataset.
The dataset must be described in \ref decl_hdf5_h5file_datasets_dataset_regex "datasets", with its whole size.

\snippet{trimleft} decl_hdf5/docs/time_series.yml decl_hdf5

The memory and dataset selections have different numbers of dimensions, but the same number of elements.
The dataset `field` of `output_0.h5` holds 3x4x6 values: the interior of the block at iteration 0, from 0 to 35, then at iteration 1, from
100 to 135, and at iteration 2, from 200 to 235.

## Describing datasets by a pattern {#decl_hdf5_ex_regex}

A key of \ref decl_hdf5_h5file_datasets_dataset_regex "datasets" is a regular expression: it describes every dataset whose whole name it
matches.
Here, every `field_<n>` dataset is created with single-precision values, to which HDF5 converts the data.

\snippet{trimleft} decl_hdf5/docs/regex.yml decl_hdf5

The datasets `field_0`, `field_1` and `field_2` of `output_0.h5` hold 6x8 single-precision values.

## Handling files and datasets that already exist {#decl_hdf5_ex_collision}

\ref decl_hdf5_h5file_collision_policy chooses what happens when a file or a dataset to write already exists; by default, it is written into.
Here, `error` refuses to overwrite the files of a previous run, one per iteration:

\snippet{trimleft} decl_hdf5/docs/no_overwrite.yml decl_hdf5

A first run writes `output_0_0.h5`, `output_0_1.h5` and `output_0_2.h5`.
A second run fails as soon as it shares `field`, with an error that tells that `output_0_0.h5` already exists.

\ref decl_hdf5_h5io_collision_policy sets it for one write: here, `field_first` keeps the value of the first iteration, while `field_last` is
overwritten at each iteration.

\snippet{trimleft} decl_hdf5/docs/first.yml decl_hdf5

The dataset `field_first` of `output_0.h5` holds the values of iteration 0, and `field_last` those of iteration 2.

## Compressing datasets {#decl_hdf5_ex_compression}

\ref decl_hdf5_h5file_deflate and \ref decl_hdf5_h5file_fletcher set the compression and checksum filters of all datasets created in the file;
\ref decl_hdf5_h5io_deflate and \ref decl_hdf5_h5io_fletcher set them for one write.
HDF5 only applies filters to chunked datasets: \ref decl_hdf5_h5io_chunking sets the size of the chunks.

\snippet{trimleft} decl_hdf5/docs/compression.yml decl_hdf5

The dataset `field` of `output_0.h5` is stored in chunks of 6x4 values, compressed with deflate at level 6, with a Fletcher32 checksum.

## Writing one file from all processes {#decl_hdf5_ex_parallel}

With a \ref decl_hdf5_h5file_communicator, all its processes access the file together, with parallel HDF5.
The \ref mpi_plugin "MPI plugin" defines `$MPI_COMM_WORLD`: it is loaded with `mpi: ~` beside `decl_hdf5` in the `plugins` section.
Each process writes its block to its own rows of a dataset that holds those of all processes.

\snippet{trimleft} decl_hdf5/docs/parallel.yml decl_hdf5

The size of the dataset selection defaults to that of the memory selection, since they have the same number of dimensions.
\ref decl_hdf5_h5io_mpio lets each process write without waiting for the others, instead of all of them together.
With 2 processes, the dataset `field` of `output.h5` holds 8x6 values: the 4 rows of process 0, from 200 to 235, then the 4 rows of process
1, from 240 to 275.

## Checkpointing and restarting {#decl_hdf5_ex_restart}

The program writes a checkpoint at each iteration with this configuration, which overwrites the previous one: the last one is always in the
file.

\snippet{trimleft} decl_hdf5/docs/checkpoint.yml decl_hdf5

A second program restarts from it.
It does not know the size of the block, so it shares 3 data in turn:

- `rank`, an `int` metadata;
- `dims`, a metadata array of 2 `int64`, to get the size of the block, ghost cells included;
- `field`, an array of `$dims[0]` x `$dims[1]` `double`, and `iter`, an `int`, together with the `restart` event, once it has allocated
  `field` with that size.

The first file description reads `dims` as soon as it is shared, with \ref decl_hdf5_h5io_size_of; the second one reads `iter` and `field` on
the `restart` event.

\snippet{trimleft} decl_hdf5/docs/restart.yml decl_hdf5

`size_of` writes 64-bit integers: the data it reads into must be an array of `int64`, with one element per dimension of the dataset.
`dims` is shared with read access too, since %PDI keeps a copy of a metadata.
The program restarts from iteration 2, with a 6x8 field.
