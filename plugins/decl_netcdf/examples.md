# Decl'NetCDF examples {#Decl_NetCDF_examples}

Each example demonstrates one feature of the \ref Decl_NetCDF_plugin "Decl'NetCDF plugin" with the `decl_netcdf` part of a specification
tree; the \ref Decl_NetCDF_reference "reference" documents each of their keys.
The whole specification trees are in `plugins/decl_netcdf/docs/` in %PDI's sources, where %PDI's tests run them and check the files they
write.

## The data {#decl_netcdf_ex_program}

All the examples but two configure the same program.
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

Decl'NetCDF writes a data as a whole, ghost cells included: unlike Decl'HDF5, it has no memory selection, and the datatype of the data it
accesses must be dense, without `subsize` or `start`.

## Writing data when it is shared {#decl_netcdf_ex_simple}

The simplest configuration names a \ref decl_netcdf_ncfile_file and the data to \ref decl_netcdf_ncfile_write_data_name "write": each one is
written to the variable of the same name each time the program shares it.

\snippet{trimleft} decl_netcdf/docs/simple.yml decl_netcdf

`simple.nc` holds a variable `size`, of 2 integers, and a variable `field`, of 6x8 values.
`field` is written at each iteration to the same variable, so the file only keeps the last iteration, ghost cells included.
Decl'NetCDF names the dimensions after the variable, `size_0`, `field_0` and `field_1`, unless they are named as in
\ref decl_netcdf_ex_dimensions.

## Naming files and variables after the data, and writing conditionally {#decl_netcdf_ex_names}

\ref decl_netcdf_ncfile_file and \ref decl_netcdf_ncio_variable are \ref spectree_expression "$-expressions": they can name a file per
process, and a variable per iteration.
\ref decl_netcdf_ncio_when restricts a write to the iterations where a condition holds.
A data to write can be given an \ref decl_netcdf_ncio "I/O description", whose keys all have a default value.

\snippet{trimleft} decl_netcdf/docs/names.yml decl_netcdf

`iter`, `rank` and `size` are metadata: %PDI keeps a copy of their last value, which the $-expressions read.
`iter` is shared with `field`, so the name of the variable is that of the current iteration.
`output_0.nc` holds the variables `size`, `field_1` and `field_2`.

## Writing on an event, to groups {#decl_netcdf_ex_event}

Without \ref decl_netcdf_ncfile_on_event, the file is opened and closed for each data written.
With it, the whole file description is executed at once when the program emits the event, here `iteration`, which the program emits while it
shares both `iter` and `field`.
A `/` in the path of a \ref decl_netcdf_ncio_variable "variable" puts it in a group, which Decl'NetCDF creates.

\snippet{trimleft} decl_netcdf/docs/event.yml decl_netcdf

`output_0.nc` holds a group per iteration, `iter0`, `iter1` and `iter2`, each with a scalar variable `iter` and a variable `field`.

## Naming dimensions, and writing attributes {#decl_netcdf_ex_dimensions}

\ref decl_netcdf_ncfile_variables_variable_path "variables" describes a variable: the names of its
\ref decl_netcdf_ncfile_variables_variable_path_dimensions "dimensions", and its
\ref decl_netcdf_ncfile_variables_variable_path_attributes "attributes".
\ref decl_netcdf_ncfile_groups_group_path "groups" gives the \ref decl_netcdf_ncfile_groups_group_path_attributes "attributes" of a group.
Each attribute is a $-expression, written when the file is opened.

\snippet{trimleft} decl_netcdf/docs/dimensions.yml decl_netcdf

`output_0.nc` holds a group `results`, with the integer attributes `nprocs`, 1, and `rank`, 0.
Its variable `field` has the dimensions `height`, 6, and `width`, 8, and the attributes `units`, the string `kelvin`, and `iteration`, 2.

## Writing records, and reading attributes back {#decl_netcdf_ex_records}

A record is written as a NetCDF compound type, whose name its datatype gives in its `+decl_netcdf.type` attribute.
This example configures a program of its own, which shares these data:

- `time`, a `double` metadata, 0.25, and `particles`, with the `save` event;
- the same data, with the `load` event, to read them back.

`particles` is declared as follows, in the `data` section of the specification tree:

\snippet{trimleft} decl_netcdf/docs/particles.yml data

An \ref decl_netcdf_ncattributes "attribute" is written from a $-expression, and read back into the data that the same $-expression
references: here `time`, written with the particles on the `save` event, and read with them on the `load` event.

\snippet{trimleft} decl_netcdf/docs/particles.yml decl_netcdf

`particles.nc` holds a compound type `particle_t`, of an `int` `id` and a `double` array `position` of 3, and a variable `particles`, of 2
of them, with a `double` attribute `time`, 0.25.
On the `load` event, the program gets the two particles and the time back.

## Writing every iteration to one variable, along an unlimited dimension {#decl_netcdf_ex_time_series}

The \ref decl_netcdf_ncfile_variables_variable_path_type "type" of a variable can differ from that of the data: here, it has one more
dimension, of size 0, which makes it `UNLIMITED`.
\ref decl_netcdf_ncio_variable_selection writes each iteration to its own step along it.

\snippet{trimleft} decl_netcdf/docs/history.yml decl_netcdf

The variable `field` of `history_0.nc` has the dimensions `time`, unlimited, with 3 steps, `height`, 6, and `width`, 8.

## Writing one file from all processes {#decl_netcdf_ex_parallel}

With a \ref decl_netcdf_ncfile_communicator, all its processes access the file together, with parallel NetCDF.
The \ref mpi_plugin "MPI plugin" defines `$MPI_COMM_WORLD`: it is loaded with `mpi: ~` beside `decl_netcdf` in the `plugins` section.
Each process writes its block to its own rows of a variable that holds those of all processes.

\snippet{trimleft} decl_netcdf/docs/parallel.yml decl_netcdf

With 2 processes, the variable `field` of `output.nc` holds 12x8 values: the 6 rows of the block of process 0, ghost cells included, then
the 6 rows of process 1.

## Compressing variables {#decl_netcdf_ex_compression}

\ref decl_netcdf_ncfile_deflate sets the compression level of all variables of the file,
\ref decl_netcdf_ncfile_variables_variable_path_deflate that of one variable, and
\ref decl_netcdf_ncfile_variables_variable_path_chunking the size of its chunks.

\snippet{trimleft} decl_netcdf/docs/compression.yml decl_netcdf

The variable `field` of `output_0.nc` is stored in chunks of 6x4 values, compressed with deflate at level 6.

## Restarting from the last step {#decl_netcdf_ex_restart}

A second program restarts from the last step of the history written in \ref decl_netcdf_ex_time_series.
It does not know the size of the history, so it shares 3 data in turn:

- `rank`, an `int` metadata;
- `dims`, a metadata array of 3 `int`, to get the size of the history: its number of steps, then the size of the field;
- `field`, an array of `$dims[1]` x `$dims[2]` `double`, once it has allocated it with that size.

Each data is read when it is shared: `dims` first, with \ref decl_netcdf_ncio_size_of, then `field`, with the size and the
\ref decl_netcdf_ncio_variable_selection "selection" that `dims` gives.
The variable is described with its type in the file, since it differs from that of the data.

\snippet{trimleft} decl_netcdf/docs/restart.yml decl_netcdf

`dims` is shared with read access too, since %PDI keeps a copy of a metadata.
The program restarts after 3 steps, with a 6x8 field.
