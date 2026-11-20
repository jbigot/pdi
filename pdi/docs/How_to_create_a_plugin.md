# How to create a plugin {#how_to_create_a_plugin}

This tutorial shows how to write the code of a plugin.
Before designing your own, read the \ref plugin_design_guidelines "plugin design guidelines", which describe how a plugin should fit the
declarative approach of %PDI.

## Plugin class {#plugin_class}

Plugin is a class, that handles shared data and triggered events to perform an I/O operations. It is dynamically linked to the user program by PDI. Behavior of each plugin is defined by specification tree in plugins subtree.

```yaml
plugins:
  example:
    ...
```
The given example will load example plugin and pass its subtree to the plugin's constructor.

The plugin has to inherit from PDI::Plugin and have a constructor with arguments
PDI::Logger&, PDI::Context& and PC_tree_t.
It forwards the logger and the context to PDI::Plugin.

### Example plugin {#example_plugin}
Example of the simplest plugin, that does nothing:

\snippet How_to_create_a_plugin/minimal_plugin.cxx example

### Adding a callback {#adding_callback}
To be notified when the user shares data (calls `PDI_share`, `PDI_expose` or `PDI_multiexpose`), a plugin registers a function (a callback)
with \ref PDI::Context::on_data, declared as:

\dontinclude pdi/context.h
\skipline on_data(

The first argument is the function to be called when user shares the data.
If the second parameter (name) is given, the function will be called only on a specified data name.
Returns a function that removes the callback from PDI::Context container.

The following steps all build up the same `example` plugin, with these includes:

\snippet How_to_create_a_plugin/example_plugin.cxx includes

Example of adding a new callback, in the constructor of the plugin, that logs the name of each data the user shares:

\snippet{trimleft} How_to_create_a_plugin/example_plugin.cxx on_data

If the user creates the following specification tree:

\snippet How_to_create_a_plugin/example_use.yml yaml

and the following program:

\snippet How_to_create_a_plugin/example_use.c example

the console displays the messages of every step of this plugin:
```
[23:44:08][PDI] *** info: Initialization successful
[23:44:08][example] *** info: Event `pdi_start_timer' called
[23:44:08][example] *** info: User has shared a data named some_integer
[23:44:08][example] *** info: some_integer = 0
[23:44:08][example] *** info: Configured event `checkpoint' called
[23:44:08][example] *** info: Event `checkpoint' called
[23:44:08][example] *** info: Event `pdi_stop_timer' called
[23:44:08][PDI] *** info: Finalization
```

### Reading and writing data {#rw_data}

A `PDI::Ref` gives no access to the data by itself.
The plugin converts it to a `PDI::Ref_r` to read the data, a `PDI::Ref_w` to write it, or a `PDI::Ref_rw` to do both, and the converted
reference is null if the share does not grant that access.
Which access to ask for follows from what the plugin was configured to do, never from what the share happens to grant: see
\ref pdg_config_decides "the design guidelines".

The type of the reference describes the data: check it before interpreting the buffer.
Example of a callback that reads an integer each time one is shared:

\snippet{trimleft} How_to_create_a_plugin/example_plugin.cxx rw_data

### Handling events {#example_events}

To be notified of events, a plugin registers a callback with `PDI::Context::on_event`, either for all events, or for one event given by name.
Like the names of data, the names of events are chosen by the user: in the specification tree above, the `on_event` key names it.


\snippet{trimleft} How_to_create_a_plugin/example_plugin.cxx events

with the handlers themselves as members of the plugin:

\snippet{trimleft} How_to_create_a_plugin/example_plugin.cxx event_handlers

### Reading scalar and array from specification tree {#reading_pc_tree}
Using the specification tree above, reading the scalar `scalar` and the array `array`:

\snippet{trimleft} How_to_create_a_plugin/example_plugin.cxx conf_scalar_array

### Reading maps from specification tree {#reading_pc_tree_2}
Using the same specification tree, reading the map `custom_subtree`:

\snippet{trimleft} How_to_create_a_plugin/example_plugin.cxx conf_map

## Creating a true plugin: POSIX plugin {#create_true_plugin}

The fastest way to learn is by example.
This section builds a "POSIX plugin" that writes data to files and reads it back, a minimal checkpointing plugin.
It does nothing a real I/O plugin would not do better, but it follows the \ref plugin_design_guidelines "plugin design guidelines", and
shows how each of them translates into code.

### Step 1: Think what your plugin will be for {#step_1}

Simple checkpointing: each data is saved to a file of its own, and read back from it on restart.
The application can also learn whether a checkpoint exists, to decide whether to restart.

The library here is POSIX file I/O, whose only concept is the file: the configuration describes files, which data to write to them or read
from them, and when.

### Step 2: Prepare your specification tree schema {#step_2}

\snippet How_to_create_a_plugin/posix.yml yaml

Each entry of `write` and `read` is a `FILE_OP`: either the name of the file alone, which always triggers, or a map with the `file` and a
`when` condition, which defaults to always.
`write: {field: field.bin}` is enough to write `field` to `field.bin` each time it is shared.

The application only shares `rank`, `iter`, `restart`, `field` and `checkpoint_found`.
It does not know which plugin uses them, and the access it grants on `field` (`PDI_OUT`, `PDI_IN` or `PDI_INOUT`) does not change what the
plugin does: the `when` conditions do.
The plugin only checks that the access is enough for what it was asked.

### Step 3: Write your plugin {#step_3}

Parse a `FILE_OP`, accepting the short form, and rejecting unknown keys with their location in the specification tree:

\snippet{trimleft} How_to_create_a_plugin/posix.cxx parse

Store the operations to execute, indexed by the name of the data that triggers them, and register a single callback for all data.
The whole configuration is parsed and checked in the constructor, so that a mistake is reported when the plugin loads.
The $-expressions, on the other hand, are kept as `PDI::Expression`s, to be evaluated when they are used:

\snippet{trimleft} How_to_create_a_plugin/posix.cxx constructor

On each share, execute the operations whose `when` holds:

\snippet{trimleft} How_to_create_a_plugin/posix.cxx data

Write the data, asking for read access to it, and reporting every failure.
The data can be sparse in memory (a subset of a larger array, for example): its type describes its layout, and is used to make a dense copy:

\snippet{trimleft} How_to_create_a_plugin/posix.cxx write

Read the data, asking for write access to it, and checking that the file holds the expected amount of data before copying it:

\snippet{trimleft} How_to_create_a_plugin/posix.cxx read

Store whether the file exists in the data the user chose.
`scalar_assign` writes the value according to the type of the data, whatever the size of the integer, and throws if it is not a scalar, so
that the plugin never writes past the end of the user's buffer:

\snippet{trimleft} How_to_create_a_plugin/posix.cxx exists

The references stay alive for the whole duration of each callback, so that no other plugin accesses the data while it is being read or
written.

### Next steps {#plugin_compile}

1. Compile:
   `g++ -std=c++20 -shared -fPIC posix.cxx -o libpdi_posix_plugin.so -lpdi -lparaconf`
2. Copy the created file next to the other plugins, or to a directory listed in `PDI_PLUGIN_PATH`.
3. Write a program that shares the data of the specification tree above, compile it and run it.
   This one computes for a few iterations, then restarts from the last checkpoint:

\snippet How_to_create_a_plugin/posix_use.c example

The next step to make this plugin a real one would be to trigger operations on events as well as on shares, with an `on_event` key, as
\ref Decl_HDF5_plugin "Decl'HDF5" does.
