# The Decl'NetCDF plugin {#Decl_NetCDF_plugin}

The Decl'NetCDF plugin reads from and writes to NetCDF files in a declarative way.

With the Decl'NetCDF plugin, you can:
1. read and write data as NetCDF variables of a NetCDF file,
2. define groups in the NetCDF file, possibly nested,
3. read and write the attributes of variables and groups, **which happens automatically when the file is opened**,
4. execute I/O operations on events,
5. execute I/O operations when data is shared (when no event is defined),
6. define `UNLIMITED` dimensions, and name the dimensions,
7. read and write only part of a NetCDF variable, by defining a hyperslab,
8. read and write in parallel, with MPI.

## Documentation {#decl_netcdf_documentation}

* \subpage Decl_NetCDF_examples "Decl'NetCDF examples" demonstrates its features one at a time, each with the part of the specification tree that configures it.
* \subpage Decl_NetCDF_reference "Decl'NetCDF configuration reference" documents each key of its configuration.
