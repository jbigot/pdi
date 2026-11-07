# Decl'HDF5 examples {#Decl_HDF5_examples}

Each example demonstrates one feature of the \ref Decl_HDF5_plugin "Decl'HDF5 plugin", with a whole specification tree that you can run as
is; the \ref Decl_HDF5_reference "reference" documents each of their keys.

## The program {#decl_hdf5_ex_program}

All the examples but the last one use the same program: only their specification tree differs.
Each MPI process owns a block of 4x6 values, with a layer of ghost cells all around it, that it updates over 3 iterations.
To make the files easy to read, each value tells where it comes from: 100 times the iteration, plus 10 times the global row, plus the column,
and the ghost cells hold -1.

```c
#include <mpi.h>
#include <pdi.h>

int main(int argc, char* argv[])
{
	MPI_Init(&argc, &argv);
	PDI_init(PC_parse_path(argv[1]));

	int rank, nprocs;
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	MPI_Comm_size(MPI_COMM_WORLD, &nprocs);
	// the size of the block of each process, without its ghost cells
	int size[2] = {4, 6};
	PDI_multi_expose("init",
	    "rank", &rank, PDI_OUT,
	    "nprocs", &nprocs, PDI_OUT,
	    "size", size, PDI_OUT,
	    NULL);

	// the block, with a layer of ghost cells all around it
	double field[4 + 2][6 + 2];
	for (int iter = 0; iter < 3; ++iter) {
		// each cell gets 100 * iter + 10 * its global row + its column,
		// the ghost cells -1
		for (int i = 0; i < 4 + 2; ++i) {
			for (int j = 0; j < 6 + 2; ++j) {
				int row = rank * 4 + i - 1, column = j - 1;
				int ghost = (i == 0 || i == 4 + 1 || j == 0 || j == 6 + 1);
				field[i][j] = ghost ? -1 : 100 * iter + 10 * row + column;
			}
		}
		PDI_multi_expose("iteration",
		    "iter", &iter, PDI_OUT,
		    "field", field, PDI_OUT,
		    NULL);
	}

	PDI_finalize();
	MPI_Finalize();
	return 0;
}
```

Compile it as described in \ref compilation_of__app, and run it with the specification tree as its argument, in an environment set up for
%PDI (see \ref running_app):

```bash
mpirun -np 1 ./main simple.yml
```

All the examples run on a single process unless they say otherwise, and the outputs shown are those of `h5dump` and `h5ls`, or the log of
the program.

## Writing data when it is shared {#decl_hdf5_ex_simple}

The simplest configuration lists the data to write: each one is written to the dataset of the same name each time the program shares it.

```yaml
metadata:
  rank: int
  nprocs: int
  size: {type: array, subtype: int, size: 2}
  iter: int
data:
  field: {type: array, subtype: double, size: ['$size[0] + 2', '$size[1] + 2']}
plugins:
  decl_hdf5:
    file: simple.h5
    write: [size, field]
```

`field` is written at each iteration, to the same dataset, so the file only keeps the last one, ghost cells included:

```text
$ h5dump simple.h5
HDF5 "simple.h5" {
GROUP "/" {
   DATASET "field" {
      DATATYPE  H5T_IEEE_F64LE
      DATASPACE  SIMPLE { ( 6, 8 ) / ( 6, 8 ) }
      DATA {
      (0,0): -1, -1, -1, -1, -1, -1, -1, -1,
      (1,0): -1, 200, 201, 202, 203, 204, 205, -1,
      (2,0): -1, 210, 211, 212, 213, 214, 215, -1,
      (3,0): -1, 220, 221, 222, 223, 224, 225, -1,
      (4,0): -1, 230, 231, 232, 233, 234, 235, -1,
      (5,0): -1, -1, -1, -1, -1, -1, -1, -1
      }
   }
   DATASET "size" {
      DATATYPE  H5T_STD_I32LE
      DATASPACE  SIMPLE { ( 2 ) / ( 2 ) }
      DATA {
      (0): 4, 6
      }
   }
}
}
```

The rest of the examples address these limits one by one.

## Naming files and datasets after the data, and writing conditionally {#decl_hdf5_ex_names}

\ref decl_hdf5_h5file_file and \ref decl_hdf5_h5io_dataset are \ref spectree_expression "$-expressions": they can name a file per process,
and a dataset per iteration.
\ref decl_hdf5_h5io_when restricts a write to the iterations where a condition holds.

```yaml
metadata:
  rank: int
  nprocs: int
  size: {type: array, subtype: int, size: 2}
  iter: int
data:
  field: {type: array, subtype: double, size: ['$size[0] + 2', '$size[1] + 2']}
plugins:
  decl_hdf5:
    file: output_${rank}.h5     # one file per process
    write:
      size: {}                  # an I/O description with the default values
      field:
        dataset: field_${iter}  # one dataset per iteration
        when: $iter > 0         # but for the first one
```

`iter`, `rank` and `size` are metadata: %PDI keeps a copy of their last value, which the $-expressions read.
`iter` is shared before `field`, so the name of the dataset is that of the current iteration.

```text
$ h5ls output_0.h5
field_1                  Dataset {6, 8}
field_2                  Dataset {6, 8}
size                     Dataset {2}
```

## Writing a data several times {#decl_hdf5_ex_several}

A sequence of I/O descriptions writes the data once for each of them, each with its own keys: here, always to the same dataset, and at the even
iterations to a dataset of its own.

```yaml
metadata:
  rank: int
  nprocs: int
  size: {type: array, subtype: int, size: 2}
  iter: int
data:
  field: {type: array, subtype: double, size: ['$size[0] + 2', '$size[1] + 2']}
plugins:
  decl_hdf5:
    file: output_${rank}.h5
    write:
      field:
        - dataset: field_last    # at each iteration
        - dataset: field_${iter} # and at the even ones
          when: $iter % 2 = 0
```

```text
$ h5ls output_0.h5
field_0                  Dataset {6, 8}
field_2                  Dataset {6, 8}
field_last               Dataset {6, 8}
```

## Writing on an event, to groups {#decl_hdf5_ex_event}

Without \ref decl_hdf5_h5file_on_event, the file is opened and closed for each data written.
With it, the whole file description is executed at once when the program emits the event, here `iteration`, which the program emits while it
shares both `iter` and `field` (see \ref PDI_multi_expose).
A `/` in the name of a dataset puts it in a group, which Decl'HDF5 creates.

```yaml
metadata:
  rank: int
  nprocs: int
  size: {type: array, subtype: int, size: 2}
  iter: int
data:
  field: {type: array, subtype: double, size: ['$size[0] + 2', '$size[1] + 2']}
plugins:
  decl_hdf5:
    file: output_${rank}.h5
    on_event: iteration          # the file is accessed once per iteration
    write:
      iter:
        dataset: iter${iter}/iter    # a group per iteration
      field:
        dataset: iter${iter}/field
```

```text
$ h5ls -r output_0.h5
/                        Group
/iter0                   Group
/iter0/field             Dataset {6, 8}
/iter0/iter              Dataset {SCALAR}
/iter1                   Group
/iter1/field             Dataset {6, 8}
/iter1/iter              Dataset {SCALAR}
/iter2                   Group
/iter2/field             Dataset {6, 8}
/iter2/iter              Dataset {SCALAR}
```

## Writing attributes {#decl_hdf5_ex_attributes}

\ref decl_hdf5_h5io_attributes_attribute_name writes attributes along with a dataset, each from a $-expression.
\ref decl_hdf5_h5io_attribute writes a data to an attribute instead of a dataset: here `iter`, to an attribute of the dataset of the current
iteration.
The file is written on the `iteration` event, so that the dataset exists when its attribute is written.

```yaml
metadata:
  rank: int
  nprocs: int
  size: {type: array, subtype: int, size: 2}
  iter: int
data:
  field: {type: array, subtype: double, size: ['$size[0] + 2', '$size[1] + 2']}
plugins:
  decl_hdf5:
    file: output_${rank}.h5
    on_event: iteration
    write:
      field:
        dataset: field_${iter}
        attributes:              # written with the dataset
          rank: $rank
      iter:
        attribute: field_${iter}#iteration  # iter, to the iteration attribute
```

```text
$ h5dump -A -d field_1 output_0.h5
HDF5 "output_0.h5" {
DATASET "field_1" {
   DATATYPE  H5T_IEEE_F64LE
   DATASPACE  SIMPLE { ( 6, 8 ) / ( 6, 8 ) }
   ATTRIBUTE "iteration" {
      DATATYPE  H5T_STD_I32LE
      DATASPACE  SCALAR
      DATA {
      (0): 1
      }
   }
   ATTRIBUTE "rank" {
      DATATYPE  H5T_STD_I32LE
      DATASPACE  SCALAR
      DATA {
      (0): 0
      }
   }
}
}
```

## Leaving the ghost cells out {#decl_hdf5_ex_memory_selection}

A \ref decl_hdf5_h5io_memory_selection writes only a part of the data: here its 4x6 interior.
A dataset that is not described in \ref decl_hdf5_h5file_datasets_dataset_regex is created with the size of the whole data, so the dataset is
described there with the size of the selection.

```yaml
metadata:
  rank: int
  nprocs: int
  size: {type: array, subtype: int, size: 2}
  iter: int
data:
  field: {type: array, subtype: double, size: ['$size[0] + 2', '$size[1] + 2']}
plugins:
  decl_hdf5:
    file: output_${rank}.h5
    when: $iter = 2
    datasets:
      field:                     # the dataset, without the ghost cells
        {type: array, subtype: double, size: ['$size[0]', '$size[1]']}
    write:
      field:
        memory_selection:        # the interior of the block
          size: ['$size[0]', '$size[1]']
          start: [1, 1]
```

```text
$ h5dump output_0.h5
HDF5 "output_0.h5" {
GROUP "/" {
   DATASET "field" {
      DATATYPE  H5T_IEEE_F64LE
      DATASPACE  SIMPLE { ( 4, 6 ) / ( 4, 6 ) }
      DATA {
      (0,0): 200, 201, 202, 203, 204, 205,
      (1,0): 210, 211, 212, 213, 214, 215,
      (2,0): 220, 221, 222, 223, 224, 225,
      (3,0): 230, 231, 232, 233, 234, 235
      }
   }
}
}
```

The datatype of the data can tell the same, with the `subsize` and `start` keys of an \ref spectree_array "array": the data is then only its
interior, wherever it is used, and the dataset created for it has its size.
This configuration writes the same file:

```yaml
metadata:
  rank: int
  nprocs: int
  size: {type: array, subtype: int, size: 2}
  iter: int
data:
  field:
    type: array
    subtype: double
    size: ['$size[0] + 2', '$size[1] + 2']
    subsize: ['$size[0]', '$size[1]']  # the part of the array that is the data
    start: [1, 1]
plugins:
  decl_hdf5:
    file: output_${rank}.h5
    when: $iter = 2
    write: [field]
```

## Writing every iteration to one dataset {#decl_hdf5_ex_time_series}

A \ref decl_hdf5_h5io_dataset_selection writes the data to a part of a larger dataset: here, each iteration to its own slice of a 3D dataset.
The dataset must be described in \ref decl_hdf5_h5file_datasets_dataset_regex, with its whole size.

```yaml
metadata:
  rank: int
  nprocs: int
  size: {type: array, subtype: int, size: 2}
  iter: int
data:
  field: {type: array, subtype: double, size: ['$size[0] + 2', '$size[1] + 2']}
plugins:
  decl_hdf5:
    file: output_${rank}.h5
    datasets:
      field:                     # 3 iterations of the interior
        {type: array, subtype: double, size: [3, '$size[0]', '$size[1]']}
    write:
      field:
        memory_selection:
          size: ['$size[0]', '$size[1]']
          start: [1, 1]
        dataset_selection:       # the slice of the current iteration
          size: [1, '$size[0]', '$size[1]']
          start: [$iter, 0, 0]
```

The memory and dataset selections have different numbers of dimensions, but the same number of elements.

```text
$ h5dump output_0.h5
HDF5 "output_0.h5" {
GROUP "/" {
   DATASET "field" {
      DATATYPE  H5T_IEEE_F64LE
      DATASPACE  SIMPLE { ( 3, 4, 6 ) / ( 3, 4, 6 ) }
      DATA {
      (0,0,0): 0, 1, 2, 3, 4, 5,
      (0,1,0): 10, 11, 12, 13, 14, 15,
      (0,2,0): 20, 21, 22, 23, 24, 25,
      (0,3,0): 30, 31, 32, 33, 34, 35,
      (1,0,0): 100, 101, 102, 103, 104, 105,
      (1,1,0): 110, 111, 112, 113, 114, 115,
      (1,2,0): 120, 121, 122, 123, 124, 125,
      (1,3,0): 130, 131, 132, 133, 134, 135,
      (2,0,0): 200, 201, 202, 203, 204, 205,
      (2,1,0): 210, 211, 212, 213, 214, 215,
      (2,2,0): 220, 221, 222, 223, 224, 225,
      (2,3,0): 230, 231, 232, 233, 234, 235
      }
   }
}
}
```

## Describing datasets by a pattern {#decl_hdf5_ex_regex}

A key of \ref decl_hdf5_h5file_datasets_dataset_regex is a regular expression: it describes every dataset whose whole name it matches.
Here, every `field_<n>` dataset is created with single-precision values, to which HDF5 converts the data.

```yaml
metadata:
  rank: int
  nprocs: int
  size: {type: array, subtype: int, size: 2}
  iter: int
data:
  field: {type: array, subtype: double, size: ['$size[0] + 2', '$size[1] + 2']}
plugins:
  decl_hdf5:
    file: output_${rank}.h5
    datasets:
      'field_[0-9]+':            # field_0, field_1...
        type: array
        subtype: float
        size: ['$size[0] + 2', '$size[1] + 2']
    write:
      field:
        dataset: field_${iter}
```

```text
$ h5dump -H -d field_2 output_0.h5
HDF5 "output_0.h5" {
DATASET "field_2" {
   DATATYPE  H5T_IEEE_F32LE
   DATASPACE  SIMPLE { ( 6, 8 ) / ( 6, 8 ) }
}
}
```

## Handling files and datasets that already exist {#decl_hdf5_ex_collision}

\ref decl_hdf5_h5file_collision_policy chooses what happens when a file or a dataset to write already exists; by default, it is written into.
Here, `error` refuses to overwrite the files of a previous run, one per iteration:

```yaml
logging: {pattern: '[%n] %v'}  # a shorter log
metadata:
  rank: int
  nprocs: int
  size: {type: array, subtype: int, size: 2}
  iter: int
data:
  field: {type: array, subtype: double, size: ['$size[0] + 2', '$size[1] + 2']}
plugins:
  decl_hdf5:
    file: output_${rank}_${iter}.h5
    collision_policy: error
    write: [field]
```

```text
$ ./main no_overwrite.yml
[Decl'HDF5] Plugin loaded successfully
[PDI] Initialization successful
[PDI] Finalization
[Decl'HDF5] Closing plugin
$ ./main no_overwrite.yml
[Decl'HDF5] Plugin loaded successfully
[PDI] Initialization successful
[PDI] System error: Unable to share `field', Unable to share `field', while sharing `field', Filename collision `output_0_0.h5': File already exists
[PDI] Finalization
[Decl'HDF5] Closing plugin
[PDI] Remaining 1 reference(s) to `iter' in PDI after program end
```

\ref decl_hdf5_h5io_collision_policy sets it for one write: here, `field_first` keeps the value of the first iteration, while `field_last` is
overwritten.

```yaml
logging: {pattern: '[%n] %v'}  # a shorter log
metadata:
  rank: int
  nprocs: int
  size: {type: array, subtype: int, size: 2}
  iter: int
data:
  field: {type: array, subtype: double, size: ['$size[0] + 2', '$size[1] + 2']}
plugins:
  decl_hdf5:
    file: output_${rank}.h5
    write:
      field:
        - dataset: field_last
        - dataset: field_first
          collision_policy: skip # written once, then left as it is
```

```text
$ ./main first.yml
[Decl'HDF5] Plugin loaded successfully
[PDI] Initialization successful
[PDI] Finalization
[Decl'HDF5] Closing plugin
$ h5dump -d field_first output_0.h5
HDF5 "output_0.h5" {
DATASET "field_first" {
   DATATYPE  H5T_IEEE_F64LE
   DATASPACE  SIMPLE { ( 6, 8 ) / ( 6, 8 ) }
   DATA {
   (0,0): -1, -1, -1, -1, -1, -1, -1, -1,
   (1,0): -1, 0, 1, 2, 3, 4, 5, -1,
   (2,0): -1, 10, 11, 12, 13, 14, 15, -1,
   (3,0): -1, 20, 21, 22, 23, 24, 25, -1,
   (4,0): -1, 30, 31, 32, 33, 34, 35, -1,
   (5,0): -1, -1, -1, -1, -1, -1, -1, -1
   }
}
}
```

## Compressing datasets {#decl_hdf5_ex_compression}

\ref decl_hdf5_h5file_deflate and \ref decl_hdf5_h5file_fletcher set the compression and checksum filters of all datasets created in the file.
HDF5 only applies filters to chunked datasets: \ref decl_hdf5_h5io_chunking sets the size of the chunks.

```yaml
metadata:
  rank: int
  nprocs: int
  size: {type: array, subtype: int, size: 2}
  iter: int
data:
  field: {type: array, subtype: double, size: ['$size[0] + 2', '$size[1] + 2']}
plugins:
  decl_hdf5:
    file: output_${rank}.h5
    deflate: 6                   # the compression level of all the datasets
    fletcher: 1                  # with a checksum
    write:
      field:
        chunking: ['$size[0] + 2', 4]  # chunks of 6x4 elements
```

```text
$ h5dump -p -H output_0.h5
HDF5 "output_0.h5" {
GROUP "/" {
   DATASET "field" {
      DATATYPE  H5T_IEEE_F64LE
      DATASPACE  SIMPLE { ( 6, 8 ) / ( 6, 8 ) }
      STORAGE_LAYOUT {
         CHUNKED ( 6, 4 )
         SIZE 136 (2.824:1 COMPRESSION)
      }
      FILTERS {
         COMPRESSION DEFLATE { LEVEL 6 }
         CHECKSUM FLETCHER32
      }
      FILLVALUE {
         FILL_TIME H5D_FILL_TIME_IFSET
         VALUE  H5D_FILL_VALUE_DEFAULT
      }
      ALLOCATION_TIME {
         H5D_ALLOC_TIME_INCR
      }
   }
}
}
```

## Writing one file from all processes {#decl_hdf5_ex_parallel}

With a \ref decl_hdf5_h5file_communicator, all its processes access the file together, with parallel HDF5.
The \ref mpi_plugin "MPI plugin" defines `$MPI_COMM_WORLD`.
Each process writes its block to its own rows of a dataset that holds those of all processes.

```yaml
metadata:
  rank: int
  nprocs: int
  size: {type: array, subtype: int, size: 2}
  iter: int
data:
  field: {type: array, subtype: double, size: ['$size[0] + 2', '$size[1] + 2']}
plugins:
  mpi: ~
  decl_hdf5:
    file: output.h5                  # one file for all processes
    communicator: $MPI_COMM_WORLD
    datasets:
      field:                         # the blocks of all processes
        type: array
        subtype: double
        size: ['$nprocs * $size[0]', '$size[1]']
    write:
      field:
        when: $iter = 2
        memory_selection:
          size: ['$size[0]', '$size[1]']
          start: [1, 1]
        dataset_selection:           # the rows of this process
          start: ['$rank * $size[0]', 0]
        mpio: INDEPENDENT            # each process writes on its own
```

The size of the dataset selection defaults to that of the memory selection, since they have the same number of dimensions.
\ref decl_hdf5_h5io_mpio lets each process write without waiting for the others, instead of all of them together.
With 2 processes:

```text
$ mpirun -np 2 ./main parallel.yml
$ h5dump output.h5
HDF5 "output.h5" {
GROUP "/" {
   DATASET "field" {
      DATATYPE  H5T_IEEE_F64LE
      DATASPACE  SIMPLE { ( 8, 6 ) / ( 8, 6 ) }
      DATA {
      (0,0): 200, 201, 202, 203, 204, 205,
      (1,0): 210, 211, 212, 213, 214, 215,
      (2,0): 220, 221, 222, 223, 224, 225,
      (3,0): 230, 231, 232, 233, 234, 235,
      (4,0): 240, 241, 242, 243, 244, 245,
      (5,0): 250, 251, 252, 253, 254, 255,
      (6,0): 260, 261, 262, 263, 264, 265,
      (7,0): 270, 271, 272, 273, 274, 275
      }
   }
}
}
```

## Checkpointing and restarting {#decl_hdf5_ex_restart}

The program writes a checkpoint at each iteration with this specification tree, which overwrites the previous one: the last one is always in
the file.

```yaml
metadata:
  rank: int
  nprocs: int
  size: {type: array, subtype: int, size: 2}
  iter: int
data:
  field: {type: array, subtype: double, size: ['$size[0] + 2', '$size[1] + 2']}
plugins:
  decl_hdf5:
    file: checkpoint_${rank}.h5
    on_event: iteration
    write: [iter, field]
```

A second program restarts from it.
It does not know the size of the block: it reads it first, with \ref decl_hdf5_h5io_size_of, allocates the block, then reads it.
`size_of` writes 64-bit integers: the data it reads into must be an array of `int64`, with one element per dimension of the dataset.

```c
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>
#include <pdi.h>

int main(int argc, char* argv[])
{
	MPI_Init(&argc, &argv);
	PDI_init(PC_parse_path(argv[1]));

	int rank;
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	PDI_expose("rank", &rank, PDI_OUT);

	// the size of the field in the checkpoint, with its ghost cells
	int64_t dims[2];
	// dims is metadata: PDI keeps a copy of it, so it needs read access
	PDI_expose("dims", dims, PDI_INOUT);
	double* field = malloc(dims[0] * dims[1] * sizeof(double));
	int iter;
	PDI_multi_expose("restart",
	    "iter", &iter, PDI_IN,
	    "field", field, PDI_IN,
	    NULL);
	printf("rank %d restarts from iteration %d, with a %dx%d field: "
	       "field[1][1] = %g\n",
	       rank, iter, (int)dims[0], (int)dims[1], field[1 * dims[1] + 1]);

	free(field);
	PDI_finalize();
	MPI_Finalize();
	return 0;
}
```

The first file description reads `dims` as soon as it is shared, the second one reads `iter` and `field` on the `restart` event, with the size
just read.

```yaml
metadata:
  rank: int
  dims: {type: array, subtype: int64, size: 2}
data:
  iter: int
  field: {type: array, subtype: double, size: ['$dims[0]', '$dims[1]']}
plugins:
  decl_hdf5:
    - file: checkpoint_${rank}.h5
      read:
        dims: {size_of: field}   # the size of the field dataset
    - file: checkpoint_${rank}.h5
      on_event: restart
      read: [iter, field]
```

With 2 processes, for both programs:

```text
$ mpirun -np 2 ./main checkpoint.yml
$ mpirun -np 2 ./restart restart.yml
rank 0 restarts from iteration 2, with a 6x8 field: field[1][1] = 200
rank 1 restarts from iteration 2, with a 6x8 field: field[1][1] = 240
```
