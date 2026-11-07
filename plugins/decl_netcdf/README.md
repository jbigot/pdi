# The Decl'NetCDF plugin {#Decl_NetCDF_plugin}

The Decl'NetCDF plugin was created to read from and write to NetCDF files in a declarative way.

Decl'NetCDF plugin allows you to:
1. Read/write data as NetCDF variables from/to NetCDF file.
2. Define groups in the NetCDF file. Groups can be nested.
3. Read/write variables and groups attributes. **Automatically read and written when openning the file.**
4. Execute input/output operations on event.
5. Execute input/output operations on data share (when the event is not defined).
6. Define `UNLIMITED` dimension and dimensions names.
7. Read/write only part of NetCDF variable by hyperslab definition.
8. Read/write in parallel mode using MPI.

## Documentation {#decl_netcdf_documentation}

* \subpage Decl_NetCDF_examples "Decl'NetCDF examples" demonstrates its features one at a time, with whole programs and specification trees.
* \subpage Decl_NetCDF_reference "Decl'NetCDF configuration reference" documents each key of its configuration.
