# The MPI plugin {#mpi_plugin}

**WARNING** This documentation is a work in progress and does not reflect the
full MPI plugin potential.

MPI plugin defines 2 types in PDI:
* `MPI_Comm` - C MPI_Comm type
* `MPI_Comm_f` - Fortran MPI_Comm type

The MPI plugin exposes several metadata that correspond to values of the MPI library:
* `MPI_COMM_WORLD_rank` - an integer, the rank of the process in `MPI_COMM_WORLD`,
* `MPI_COMM_WORLD` - an `MPI_Comm`, `MPI_COMM_WORLD` for C,
* `MPI_COMM_SELF` - an `MPI_Comm`, `MPI_COMM_SELF` for C,
* `MPI_COMM_NULL` - an `MPI_Comm`, `MPI_COMM_NULL` for C,
* `MPI_COMM_WORLD_F` - an `MPI_Comm_f` (an integer), `MPI_COMM_WORLD` for Fortran,
* `MPI_COMM_SELF_F` - an `MPI_Comm_f` (an integer), `MPI_COMM_SELF` for Fortran,
* `MPI_COMM_NULL_F` - an `MPI_Comm_f` (an integer), `MPI_COMM_NULL` for Fortran.

All above metadata are available from PDI_init call to PDI_finalize call.

The plugin can also transtype communicators between C and Fortran: each time a source communicator is shared, the transtyped communicator is
shared under a destination name.
When the source is released, the value of the destination is transtyped back into it if it was shared with write access, then the destination
is released.

## Example of an MPI configuration {#mpi_configuration}

```yaml
data:
  mpi_comm: MPI_Comm
  mpi_comm_f: MPI_Comm_f
plugins:
  mpi:
    Comm_c2f:
      mpi_comm: m_mpi_comm_f
    Comm_f2c:
      mpi_comm_f: m_mpi_comm
```

## `@mpi` {#mpi_config}

Empty (`~`) or a mapping with the keys below.
Use it, as the value of `plugins.mpi` (see \ref spectree_pdi_plugins_plugin_name), to load and configure the MPI plugin.

**Example:**

```yaml
plugins:
  mpi: ~                  # <- no configuration: only the MPI metadata and types
```


**Example:**

```yaml
plugins:
  mpi:                    # <- with communicators to transtype
    Comm_c2f:
      my_comm: my_comm_f
```


### `@mpi.Comm_c2f.<data_name>` {#mpi_Comm_c2f_data_name}

A scalar.
Use `Comm_c2f.<data_name>` to share, under this name, the Fortran communicator transtyped from the C communicator named `<data_name>`.

**Example:**

```yaml
Comm_c2f:
  my_comm: my_comm_f      # <- each time my_comm (an MPI_Comm) is shared, its Fortran handle is shared as my_comm_f
```


### `@mpi.Comm_f2c.<data_name>` {#mpi_Comm_f2c_data_name}

A scalar.
Use `Comm_f2c.<data_name>` to share, under this name, the C communicator transtyped from the Fortran communicator named `<data_name>`.

**Example:**

```yaml
Comm_f2c:
  my_comm_f: my_comm      # <- each time my_comm_f (an MPI_Comm_f) is shared, its C handle is shared as my_comm
```


### `@mpi.transtype.<data_name>` {#mpi_transtype_data_name}

*Deprecated*, use \ref mpi_Comm_c2f_data_name and \ref mpi_Comm_f2c_data_name instead.
A scalar.
Use `transtype.<data_name>` to share, under this name, the communicator transtyped from the communicator named `<data_name>`.
The direction of the transtyping depends on the type of `<data_name>`: C to Fortran for an `MPI_Comm`, Fortran to C otherwise.

**Example:**

```yaml
transtype:
  my_comm: my_comm_f      # <- deprecated: Comm_c2f: {my_comm: my_comm_f}
```


### `@mpi.logging` {#mpi_logging}

*Optional.*
A \ref spectree_logging.
Use `logging` to configure the logger of the plugin, see \ref spectree_pdi_plugins_plugin_name_logging.

**Example:**

```yaml
logging: debug            # <- the logger of the MPI plugin
```


