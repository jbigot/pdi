# The MPI plugin {#mpi_plugin}

**WARNING** This documentation is a work in progress and does not reflect the
full potential of the MPI plugin.

The MPI plugin defines two types in PDI:
* `MPI_Comm` - the C `MPI_Comm` type,
* `MPI_Comm_f` - the Fortran communicator type, an integer.

The MPI plugin exposes several metadata that correspond to values of the MPI library:
* `MPI_COMM_WORLD_rank` - an integer, the rank of the process in `MPI_COMM_WORLD`,
* `MPI_COMM_WORLD` - an `MPI_Comm`, `MPI_COMM_WORLD` for C,
* `MPI_COMM_SELF` - an `MPI_Comm`, `MPI_COMM_SELF` for C,
* `MPI_COMM_NULL` - an `MPI_Comm`, `MPI_COMM_NULL` for C,
* `MPI_COMM_WORLD_F` - an `MPI_Comm_f` (an integer), `MPI_COMM_WORLD` for Fortran,
* `MPI_COMM_SELF_F` - an `MPI_Comm_f` (an integer), `MPI_COMM_SELF` for Fortran,
* `MPI_COMM_NULL_F` - an `MPI_Comm_f` (an integer), `MPI_COMM_NULL` for Fortran.

All the metadata above are available from the call to `PDI_init` to the call to `PDI_finalize`.

The plugin can also transtype communicators between C and Fortran: each time a source communicator is shared, the transtyped communicator is
shared under a destination name.
When the source is released, the value of the destination is transtyped back into it if it was shared with write access, then the destination
is released.

## Example of an MPI configuration {#mpi_configuration}

\snippet{trimleft} mpi/docs/reference.yml mpi_configuration

## `@mpi` {#mpi_config}

Empty (`~`) or a mapping with the keys below.
Use it, as the value of `plugins.mpi` (see \ref spectree_pdi_plugins_plugin_name), to load and configure the MPI plugin.

**Example:**

\snippet{trimleft} mpi/docs/reference.yml mpi_config


**Example:**

\snippet{trimleft} mpi/docs/reference.yml mpi_config_mapping


### `@mpi.Comm_c2f.<data_name>` {#mpi_Comm_c2f_data_name}

A scalar.
Use `Comm_c2f.<data_name>` to share, under this name, the Fortran communicator transtyped from the C communicator named `<data_name>`.

**Example:**

\snippet{trimleft} mpi/docs/reference.yml mpi_Comm_c2f_data_name


### `@mpi.Comm_f2c.<data_name>` {#mpi_Comm_f2c_data_name}

A scalar.
Use `Comm_f2c.<data_name>` to share, under this name, the C communicator transtyped from the Fortran communicator named `<data_name>`.

**Example:**

\snippet{trimleft} mpi/docs/reference.yml mpi_Comm_f2c_data_name


### `@mpi.transtype.<data_name>` {#mpi_transtype_data_name}

*Deprecated*, use \ref mpi_Comm_c2f_data_name and \ref mpi_Comm_f2c_data_name instead.
A scalar.
Use `transtype.<data_name>` to share, under this name, the communicator transtyped from the communicator named `<data_name>`.
The direction of the transtyping depends on the type of `<data_name>`: C to Fortran for an `MPI_Comm`, Fortran to C otherwise.

**Example:**

\snippet{trimleft} mpi/docs/reference.yml mpi_transtype_data_name


### `@mpi.logging` {#mpi_logging}

*Optional.*
A \ref spectree_logging.
Use `logging` to configure the logger of the plugin, see \ref spectree_pdi_plugins_plugin_name_logging.

**Example:**

\snippet{trimleft} mpi/docs/reference.yml mpi_logging


