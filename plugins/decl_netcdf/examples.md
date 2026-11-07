# Decl'NetCDF examples {#Decl_NetCDF_examples}

Each example demonstrates one feature of the \ref Decl_NetCDF_plugin "Decl'NetCDF plugin", with a whole specification tree that you can run
as is; the \ref Decl_NetCDF_reference "reference" documents each of their keys.

## The program {#decl_netcdf_ex_program}

All the examples but two use the same program: only their specification tree differs.
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

All the examples run on a single process unless they say otherwise, and the outputs shown are those of `ncdump`.

Decl'NetCDF writes a data as a whole, ghost cells included: unlike Decl'HDF5, it has no memory selection, and the datatype of the data it
accesses must be dense, without `subsize` or `start`.

## Writing data when it is shared {#decl_netcdf_ex_simple}

The simplest configuration lists the data to write: each one is written to the variable of the same name each time the program shares it.

```yaml
metadata:
  rank: int
  nprocs: int
  size: {type: array, subtype: int, size: 2}
  iter: int
data:
  field: {type: array, subtype: double, size: ['$size[0] + 2', '$size[1] + 2']}
plugins:
  decl_netcdf:
    file: simple.nc
    write: [size, field]
```

`field` is written at each iteration, to the same variable, so the file only keeps the last one.
Decl'NetCDF names the dimensions after the variable, unless they are named as in \ref decl_netcdf_ex_dimensions.

```text
$ ncdump simple.nc
netcdf simple {
dimensions:
	size_0 = 2 ;
	field_0 = 6 ;
	field_1 = 8 ;
variables:
	int size(size_0) ;
	double field(field_0, field_1) ;
data:

 size = 4, 6 ;

 field =
  -1, -1, -1, -1, -1, -1, -1, -1,
  -1, 200, 201, 202, 203, 204, 205, -1,
  -1, 210, 211, 212, 213, 214, 215, -1,
  -1, 220, 221, 222, 223, 224, 225, -1,
  -1, 230, 231, 232, 233, 234, 235, -1,
  -1, -1, -1, -1, -1, -1, -1, -1 ;
}
```

## Naming files and variables after the data, and writing conditionally {#decl_netcdf_ex_names}

\ref decl_netcdf_ncfile_file and \ref decl_netcdf_ncio_variable are \ref spectree_expression "$-expressions": they can name a file per
process, and a variable per iteration.
\ref decl_netcdf_ncio_when restricts a write to the iterations where a condition holds.

```yaml
metadata:
  rank: int
  nprocs: int
  size: {type: array, subtype: int, size: 2}
  iter: int
data:
  field: {type: array, subtype: double, size: ['$size[0] + 2', '$size[1] + 2']}
plugins:
  decl_netcdf:
    file: output_${rank}.nc     # one file per process
    write:
      size: {}                  # an I/O description with the default values
      field:
        variable: field_${iter} # one variable per iteration
        when: $iter > 0         # but for the first one
```

`iter`, `rank` and `size` are metadata: %PDI keeps a copy of their last value, which the $-expressions read.
`iter` is shared before `field`, so the name of the variable is that of the current iteration.

```text
$ ncdump -h output_0.nc
netcdf output_0 {
dimensions:
	size_0 = 2 ;
	field_1_0 = 6 ;
	field_1_1 = 8 ;
	field_2_0 = 6 ;
	field_2_1 = 8 ;
variables:
	int size(size_0) ;
	double field_1(field_1_0, field_1_1) ;
	double field_2(field_2_0, field_2_1) ;
}
```

## Writing on an event, to groups {#decl_netcdf_ex_event}

Without \ref decl_netcdf_ncfile_on_event, the file is opened and closed for each data written.
With it, the whole file description is executed at once when the program emits the event, here `iteration`, which the program emits while it
shares both `iter` and `field` (see \ref PDI_multi_expose).
A `/` in the path of a variable puts it in a group, which Decl'NetCDF creates.

```yaml
metadata:
  rank: int
  nprocs: int
  size: {type: array, subtype: int, size: 2}
  iter: int
data:
  field: {type: array, subtype: double, size: ['$size[0] + 2', '$size[1] + 2']}
plugins:
  decl_netcdf:
    file: output_${rank}.nc
    on_event: iteration          # the file is accessed once per iteration
    write:
      iter:
        variable: iter${iter}/iter   # a group per iteration
      field:
        variable: iter${iter}/field
```

```text
$ ncdump -h output_0.nc
netcdf output_0 {

group: iter0 {
  dimensions:
  	field_0 = 6 ;
  	field_1 = 8 ;
  variables:
  	double field(field_0, field_1) ;
  	int iter ;
  } // group iter0

group: iter1 {
  dimensions:
  	field_0 = 6 ;
  	field_1 = 8 ;
  variables:
  	double field(field_0, field_1) ;
  	int iter ;
  } // group iter1

group: iter2 {
  dimensions:
  	field_0 = 6 ;
  	field_1 = 8 ;
  variables:
  	double field(field_0, field_1) ;
  	int iter ;
  } // group iter2
}
```

## Naming dimensions, and writing attributes {#decl_netcdf_ex_dimensions}

\ref decl_netcdf_ncfile_variables_variable_path describes a variable: the names of its dimensions, and its attributes.
\ref decl_netcdf_ncfile_groups_group_path gives the attributes of a group.
Each attribute is a $-expression, written when the file is opened.

```yaml
metadata:
  rank: int
  nprocs: int
  size: {type: array, subtype: int, size: 2}
  iter: int
data:
  field: {type: array, subtype: double, size: ['$size[0] + 2', '$size[1] + 2']}
plugins:
  decl_netcdf:
    file: output_${rank}.nc
    groups:
      results:
        attributes:              # the attributes of the results group
          nprocs: $nprocs
          rank: $rank
    variables:
      results/field:
        dimensions: [height, width]
        attributes:              # the attributes of the field variable
          units: kelvin
          iteration: $iter
    write:
      field:
        variable: results/field
        when: $iter = 2
```

```text
$ ncdump -h output_0.nc
netcdf output_0 {

group: results {
  dimensions:
  	height = 6 ;
  	width = 8 ;
  variables:
  	double field(height, width) ;
  		field:units = "kelvin" ;
  		field:iteration = 2 ;

  // group attributes:
  		:nprocs = 1 ;
  		:rank = 0 ;
  } // group results
}
```

## Writing records, and reading attributes back {#decl_netcdf_ex_records}

A record is written as a NetCDF compound type, whose name its datatype gives in its `+decl_netcdf.type` attribute.
An attribute is written from a $-expression, and read back into the data that the same $-expression references: here `time`, written with the
particles on the `save` event, and read with them on the `load` event.
This example has a program of its own.

```c
#include <stdio.h>
#include <pdi.h>

struct particle {
	int id;
	double position[3];
};

int main(int argc, char* argv[])
{
	PDI_init(PC_parse_path(argv[1]));

	struct particle particles[2] = {{1, {0.5, 1.5, 2.5}}, {2, {3., 4., 5.}}};
	double time = 0.25;
	PDI_multi_expose("save",
	    "time", &time, PDI_OUT,
	    "particles", particles, PDI_OUT,
	    NULL);

	struct particle loaded[2] = {{0}};
	double loaded_time = 0;
	PDI_multi_expose("load",
	    "time", &loaded_time, PDI_INOUT,
	    "particles", loaded, PDI_IN,
	    NULL);
	printf("at time %g, particle %d was at (%g, %g, %g)\n",
	       loaded_time, loaded[1].id, loaded[1].position[0],
	       loaded[1].position[1], loaded[1].position[2]);

	PDI_finalize();
	return 0;
}
```

```yaml
metadata:
  time: double
data:
  particles:
    type: array
    size: 2
    subtype:
      type: struct
      +decl_netcdf.type: particle_t  # the name of the compound type
      members:
        - id: int
        - position: {type: array, subtype: double, size: 3}
plugins:
  decl_netcdf:
    - file: particles.nc
      on_event: save
      variables:
        particles:
          attributes:
            time: $time          # written from time
      write: [particles]
    - file: particles.nc
      on_event: load
      variables:
        particles:
          attributes:
            time: $time          # read into time
      read: [particles]
```

```text
$ ./particles particles.yml
at time 0.25, particle 2 was at (3, 4, 5)
$ ncdump particles.nc
netcdf particles {
types:
  compound particle_t {
    int id ;
    double position(3) ;
  }; // particle_t
dimensions:
	particles_0 = 2 ;
variables:
	particle_t particles(particles_0) ;
		particles:time = 0.25 ;
data:

 particles = {1, {0.5, 1.5, 2.5}}, {2, {3, 4, 5}} ;
}
```

## Writing every iteration to one variable, along an unlimited dimension {#decl_netcdf_ex_time_series}

The \ref decl_netcdf_ncfile_variables_variable_path_type of a variable can differ from that of the data: here, it has one more dimension, of size
0, which makes it `UNLIMITED`.
\ref decl_netcdf_ncio_variable_selection writes each iteration to its own step along it.

```yaml
metadata:
  rank: int
  nprocs: int
  size: {type: array, subtype: int, size: 2}
  iter: int
data:
  field: {type: array, subtype: double, size: ['$size[0] + 2', '$size[1] + 2']}
plugins:
  decl_netcdf:
    file: history_${rank}.nc
    variables:
      field:
        type: array              # one more dimension than the data,
        subtype: double
        size: [0, '$size[0] + 2', '$size[1] + 2']   # an UNLIMITED one
        dimensions: [time, height, width]
    write:
      field:
        variable_selection:      # the step of the current iteration
          start: [$iter, 0, 0]
          subsize: [1, '$size[0] + 2', '$size[1] + 2']
```

```text
$ ncdump -h history_0.nc
netcdf history_0 {
dimensions:
	time = UNLIMITED ; // (3 currently)
	height = 6 ;
	width = 8 ;
variables:
	double field(time, height, width) ;
}
```

## Writing one file from all processes {#decl_netcdf_ex_parallel}

With a \ref decl_netcdf_ncfile_communicator, all its processes access the file together, with parallel NetCDF.
The \ref mpi_plugin "MPI plugin" defines `$MPI_COMM_WORLD`.
Each process writes its block to its own rows of a variable that holds those of all processes.

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
  decl_netcdf:
    file: output.nc                  # one file for all processes
    communicator: $MPI_COMM_WORLD
    variables:
      field:                         # the blocks of all processes
        type: array
        subtype: double
        size: ['$nprocs * ($size[0] + 2)', '$size[1] + 2']
        dimensions: [height, width]
    write:
      field:
        when: $iter = 2
        variable_selection:          # the rows of this process
          start: ['$rank * ($size[0] + 2)', 0]
          subsize: ['$size[0] + 2', '$size[1] + 2']
```

With 2 processes:

```text
$ mpirun -np 2 ./main parallel.yml
$ ncdump output.nc
netcdf output {
dimensions:
	height = 12 ;
	width = 8 ;
variables:
	double field(height, width) ;
data:

 field =
  -1, -1, -1, -1, -1, -1, -1, -1,
  -1, 200, 201, 202, 203, 204, 205, -1,
  -1, 210, 211, 212, 213, 214, 215, -1,
  -1, 220, 221, 222, 223, 224, 225, -1,
  -1, 230, 231, 232, 233, 234, 235, -1,
  -1, -1, -1, -1, -1, -1, -1, -1,
  -1, -1, -1, -1, -1, -1, -1, -1,
  -1, 240, 241, 242, 243, 244, 245, -1,
  -1, 250, 251, 252, 253, 254, 255, -1,
  -1, 260, 261, 262, 263, 264, 265, -1,
  -1, 270, 271, 272, 273, 274, 275, -1,
  -1, -1, -1, -1, -1, -1, -1, -1 ;
}
```

## Compressing variables {#decl_netcdf_ex_compression}

\ref decl_netcdf_ncfile_deflate sets the compression level of all variables of the file, and
\ref decl_netcdf_ncfile_variables_variable_path_chunking the size of the chunks of one of them.

```yaml
metadata:
  rank: int
  nprocs: int
  size: {type: array, subtype: int, size: 2}
  iter: int
data:
  field: {type: array, subtype: double, size: ['$size[0] + 2', '$size[1] + 2']}
plugins:
  decl_netcdf:
    file: output_${rank}.nc
    deflate: 6                   # the compression level of all the variables
    variables:
      field:
        chunking: ['$size[0] + 2', 4]  # chunks of 6x4 elements
    write: [field]
```

```text
$ ncdump -h -s output_0.nc
netcdf output_0 {
dimensions:
	field_0 = 6 ;
	field_1 = 8 ;
variables:
	double field(field_0, field_1) ;
		field:_Storage = "chunked" ;
		field:_ChunkSizes = 6, 4 ;
		field:_Shuffle = "true" ;
		field:_DeflateLevel = 6 ;
		field:_Endianness = "little" ;

// global attributes:
		:_NCProperties = "version=2,netcdf=4.10.0,hdf5=1.14.6" ;
		:_SuperblockVersion = 2 ;
		:_IsNetcdf4 = 1 ;
		:_Format = "netCDF-4" ;
}
```

## Restarting from the last step {#decl_netcdf_ex_restart}

A second program restarts from the last step of the history written in \ref decl_netcdf_ex_time_series.
It does not know the size of the history: it reads it first, with \ref decl_netcdf_ncio_size_of, allocates the block, then reads the last step.

```c
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

	// the size of the history: steps, then rows and columns of the field
	int dims[3];
	// dims is metadata: PDI keeps a copy of it, so it needs read access
	PDI_expose("dims", dims, PDI_INOUT);
	double* field = malloc(dims[1] * dims[2] * sizeof(double));
	PDI_expose("field", field, PDI_IN);
	printf("rank %d restarts after %d steps, with a %dx%d field: "
	       "field[1][1] = %g\n",
	       rank, dims[0], dims[1], dims[2], field[1 * dims[2] + 1]);

	free(field);
	PDI_finalize();
	MPI_Finalize();
	return 0;
}
```

Each data is read when it is shared: `dims` first, then `field`, with the size and the selection that `dims` gives.
The variable is described with its type in the file, since it differs from that of the data.

```yaml
metadata:
  rank: int
  dims: {type: array, subtype: int, size: 3}
data:
  field: {type: array, subtype: double, size: ['$dims[1]', '$dims[2]']}
plugins:
  decl_netcdf:
    file: history_${rank}.nc
    variables:
      field:                     # the history, in the file
        type: array
        subtype: double
        size: ['$dims[0]', '$dims[1]', '$dims[2]']
    read:
      dims:
        size_of: field           # the size of the field variable
      field:
        variable_selection:      # its last step
          start: ['$dims[0] - 1', 0, 0]
          subsize: [1, '$dims[1]', '$dims[2]']
```

With 2 processes, after the history is written:

```text
$ mpirun -np 2 ./main history.yml
$ mpirun -np 2 ./restart restart.yml
rank 0 restarts after 3 steps, with a 6x8 field: field[1][1] = 200
rank 1 restarts after 3 steps, with a 6x8 field: field[1][1] = 240
```
