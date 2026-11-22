# How to create a plugin {#how_to_create_a_plugin}

This tutorial shows how to write the code of a plugin.
The \ref plugin_design_guidelines "plugin design guidelines" complement it with how to design the interface of a plugin, its subtree of the specification tree.

The code of this tutorial is that of the `example`, `minimal` and `posix` plugins of the %PDI distribution, in `plugins/example` and `plugins/posix`.
They are built and tested with %PDI, but never installed.

## CMake plumbing {#plugin_cmake}

The first step to create a plugin is to create its CMake project.
Plugins are CMake `MODULE` libraries that link to the `PDI::PDI_plugins` target, as the POSIX plugin, built in the last section, does:

```cmake
project(pdi_posix_plugin LANGUAGES C CXX)

find_package(PDI REQUIRED COMPONENTS plugins)

add_library(pdi_posix_plugin MODULE posix.cxx)
target_link_libraries(pdi_posix_plugin PDI::PDI_plugins)
set_target_properties(pdi_posix_plugin PROPERTIES CXX_VISIBILITY_PRESET hidden)
```

%PDI loads the plugin named `<name>` in the specification tree from a file named `libpdi_<name>_plugin.so`, so the target is named `pdi_<name>_plugin`.
`PDI::PDI_plugins` brings the include directories, the libraries and the C++20 support the plugin API requires.
The symbols %PDI looks for are explicitly exported, so the others can be hidden.

A plugin meant to be used is also installed, in the directory where %PDI installed its own plugins, as the plugins distributed with %PDI, such as \ref trace_plugin "Trace", do:

```cmake
install(TARGETS pdi_posix_plugin LIBRARY DESTINATION "${PDI_DEFAULT_PLUGINDIR}")
```

`PDI_DEFAULT_PLUGINDIR` is usually relative to the installation prefix: installing the plugin with the same prefix as %PDI puts it next to the plugins of %PDI.

## Plugin class {#plugin_class}

Plugins are classes that handle shared buffers and triggered events.
They are dynamically loaded into the application by %PDI.
Their behavior is guided by their dedicated subtree in the specification tree.

### Minimal plugin {#example_plugin}

The simplest possible plugin, one that does nothing, looks as follows:

\snippet example/minimal_plugin.cxx example

It is a class named `<name>_plugin` that inherits from \ref PDI::Plugin.
Its constructor takes three arguments: a `PDI::Logger&`, a `PDI::Context&` and a `PC_tree_t`, its subtree of the specification tree.
It forwards the logger and the context to \ref PDI::Plugin, which makes them available through its `logger()` and `context()` member functions.
The class is then registered as a plugin with `PDI_PLUGIN(<name>)`.
It sits in an anonymous namespace, as the plugins distributed with %PDI do, so that its name never clashes with that of a class of another plugin or of the application.

`PDI_PLUGIN` also looks for two optional static member functions of the class:

* `pretty_name()` returns the name of the plugin in the logs, such as Decl'HDF5 for `decl_hdf5`, and defaults to `<name>`;
* `dependencies()` returns a pair of sets of plugin names: the plugins that must be in the specification tree too, and those that, when they are, must be loaded first.

The rest of this section builds the `example` plugin up, one callback at a time.

### Adding callbacks {#adding_callback}

Plugins react at various stages of the life cycle of the application: when it emits an event or shares a buffer.
To do so, plugins register callbacks to be notified.

To be notified when the application emits an event (with \ref PDI_event), plugins register a callback with \ref PDI::Context::on_event.
The first argument is the function to call when the event is emitted (the callback), which takes the name of the event as parameter.
If the second, optional, argument is provided, the callback is only triggered for the event with that name.
The returned value is a function that unregisters the callback.

This can be used, for example, to log every event the application emits.
To do so, first include the logging header:

\snippet example/example_plugin.cxx include_logger

Then, register the callback itself:

\snippet{trimleft} example/example_plugin.cxx on_event

Similarly, to be notified when the application shares a buffer (with \ref PDI_share, \ref PDI_expose, or \ref PDI_multi_expose), plugins register a callback with \ref PDI::Context::on_data.
The first argument is the callback, which takes the name of the buffer and a \ref PDI::Ref to its content.
The second, optional, argument restricts the callback to the buffer with that name, and the returned value is a function that unregisters the callback.

For example, the following callback logs the name of each buffer the application shares:

\snippet{trimleft} example/example_plugin.cxx on_data

The context offers other callbacks, documented with \ref PDI::Context, such as `on_init` once every plugin is loaded and the data of the specification tree declared, `on_data_remove` when a buffer is reclaimed, and `on_missing_data` when the value of a data that is not shared is accessed, which gives plugins a chance to share it.

%PDI removes every callback when it is finalized, before it destroys the plugins, so the callbacks may capture `this`.
A plugin only needs to call the function `on_event` or `on_data` returns to stop being notified earlier.

### Using the specification subtree {#plugin_subtree}

More realistic plugins let users guide their behavior from the specification tree.
For example, a plugin may only react to an event whose name the user specifies:

\snippet example/tests/example_use.yml yaml

The plugin reads its subtree with the paraconf wrappers of %PDI:

\snippet example/example_plugin.cxx include_paraconf

It reads the name of the event from its subtree, and registers a callback for that event only:

\snippet{trimleft} example/example_plugin.cxx user_event

with the handler itself as a member of the plugin:

\snippet{trimleft} example/example_plugin.cxx event_handlers

The plugins are constructed before the data and metadata of the specification tree are declared, and before the application shares anything.
The constructor can thus read the subtree of the plugin, but not evaluate its $-expressions: that has to wait until the callbacks are triggered, or at least until `on_init`.

### Reading and writing data {#rw_data}

A `PDI::Ref` gives no access to the data by itself.
Plugins convert it to a `PDI::Ref_r` to read the data, a `PDI::Ref_w` to write it, or a `PDI::Ref_rw` to do both, and the converted reference is null if the share does not grant that access.
Which access to ask for follows from what plugins were configured to do, never from what the share happens to grant: see \ref pdg_config_decides "the design guidelines".

The type of the reference describes the data: check it before interpreting the buffer.
Here is a callback that reads an integer each time one is shared:

\snippet{trimleft} example/example_plugin.cxx rw_data

### Running the example {#example_run}

The following program loads the plugin with the specification tree above, given as its argument, shares `some_integer` and emits the `checkpoint` event:

\snippet example/tests/example_use.c example

With all the callbacks above registered, it displays:

\snippet example/tests/example_use.output output

The `pdi_start_timer` and `pdi_stop_timer` events are emitted by %PDI itself, at initialization and finalization, for the \ref timer_plugin "Timer" plugin.

### Reading scalars and arrays from the specification tree {#reading_pc_tree}

With the specification tree above, reading the scalar `scalar` and the array `array` looks as follows:

\snippet{trimleft} example/example_plugin.cxx conf_scalar_array

### Reading maps from the specification tree {#reading_pc_tree_2}

With the same specification tree, reading the map `custom_subtree` looks as follows:

\snippet{trimleft} example/example_plugin.cxx conf_map

The example plugin reads these values to show how, but does nothing with them, and accepts any other key.
A real plugin rejects the keys it does not know, as the POSIX plugin below does.

### Reporting errors {#plugin_errors}

Plugins report errors by throwing a `PDI::Error`, which %PDI turns into an error status for the application, according to its error handling settings.
`pdi/error.h` defines one subclass per kind of error, each setting the status the application receives:

* `PDI::Spectree_error` for a mistake in the specification tree, which takes the node at fault so that the message gives its location;
* `PDI::Permission_error` when a share does not grant the access the plugin needs;
* `PDI::Type_error` when the type of a data does not suit what the plugin does with it;
* `PDI::Value_error` when a value is wrong, such as a file holding less data than expected;
* `PDI::System_error` when a library or a system call fails;
* `PDI::Plugin_error` for other failures of the plugin.

Each of them takes a format string and its arguments, as the logger does.

### Timing operations {#plugin_timers}

A `PDI::Timer_event_handler` emits the `<plugin_name>_start_timer` event when it is constructed and the matching `<plugin_name>_stop_timer` event when it is destroyed.
Declared at the start of an operation, it lets the \ref timer_plugin "Timer" plugin measure how long the operation takes, as \ref Decl_HDF5_plugin "Decl'HDF5" and \ref Decl_NetCDF_plugin "Decl'NetCDF" do.

## Creating a true plugin: the POSIX plugin {#create_true_plugin}

The fastest way to learn is by example.
This section builds a "POSIX plugin" that writes data to files and reads it back: a minimal checkpointing plugin.
It does nothing a real I/O plugin would not do better, but it follows the \ref plugin_design_guidelines "plugin design guidelines", and shows how each of them translates into code.

### Step 1: Decide what your plugin is for {#step_1}

Simple checkpointing: each data is saved to a file of its own, and read back from it on restart.
The application can also learn whether a checkpoint exists, to decide whether to restart.

The library here is POSIX file I/O, whose only concept is the file: the specification tree describes files, which data to write to them or read from them, and when.

### Step 2: Design your specification tree {#step_2}

\snippet posix/tests/posix.yml yaml

Each entry of `write` and `read` maps the name of a data to a file operation: either the name of the file alone, or a map with the `file`, an optional `when` condition, which defaults to always, and an optional `on_event`.
Without `on_event`, the operation is executed each time the data is shared.
With it, the operation is executed on each of the events it names, a single one or a list, and the data must be shared at that time.
`write: {field: field.bin}` is thus enough to write `field` to `field.bin` each time it is shared.
Each entry of `exists` maps the name of a data to the name of a file, and stores in that data whether the file exists.

The application only shares `rank`, `iter`, `restart`, `field` and `checkpoint_found`, and emits `newiter`.
It does not know which plugin uses them, and the access it grants on `field` (`PDI_OUT`, `PDI_IN` or `PDI_INOUT`) does not change what the plugin does: the `when` conditions do.
The plugin only checks that the access is enough for what it was asked to do.

### Step 3: Write your plugin {#step_3}

Parse a file operation, accepting the short form, and rejecting unknown keys with their location in the specification tree:

\snippet{trimleft} posix/posix.cxx parse

Store the operations to execute on share, indexed by the name of the data that triggers them, and register a single callback for all data.
The whole subtree is parsed and checked in the constructor, so that a mistake is reported when the plugin loads.
The `logging` key is the only one not to reject: %PDI handles it for every plugin.
The $-expressions, on the other hand, are kept as `PDI::Expression` objects, to be evaluated when they are used.
`pretty_name()` names the plugin `POSIX` in the logs:

\snippet{trimleft} posix/posix.cxx constructor

The operations triggered by events get a callback each, which accesses the data through the context, since no reference comes with an event:

\snippet{trimleft} posix/posix.cxx add_operation

On each share, execute the operations whose `when` holds:

\snippet{trimleft} posix/posix.cxx data

Write the data, asking for read access to it, and reporting every failure.
The data can be sparse in memory (a subset of a larger array, for example): its type describes its layout, and is used to make a dense copy:

\snippet{trimleft} posix/posix.cxx write

Read the data, asking for write access to it, and checking that the file holds the expected amount of data before copying it:

\snippet{trimleft} posix/posix.cxx read

Store whether the file exists in the data the user chose.
`scalar_assign` writes the value according to the type of the data, whatever the size of the integer, and throws if it is not a scalar, so that the plugin never writes past the end of the user's buffer:

\snippet{trimleft} posix/posix.cxx exists

The references stay alive for the whole duration of each operation, so that no other plugin modifies the data while it is being read or written.
Reading a checkpoint back is logged at the `info` level, since it changes the course of the run, and the other operations at the `debug` level, to diagnose a specification tree that does not behave as expected.

### Step 4: Build and use your plugin {#plugin_compile}

Build the plugin with the CMake project of the \ref plugin_cmake "first section", and install it with the same prefix as %PDI:

```sh
cmake -B build -DCMAKE_INSTALL_PREFIX=<PDI prefix>
cmake --build build
cmake --install build
```

Alternatively, leave it where it was built, and give %PDI its directory, either in the `PDI_PLUGIN_PATH` environment variable or in the `plugin_path` key of the specification tree.

Then, write a program that shares the data of the specification tree above, compile it and run it.
This one computes for a few iterations, then restarts from the last checkpoint:

\snippet posix/tests/posix_use.c example

It displays the line the plugin logs at the `info` level when it reads the checkpoint back:

\snippet posix/tests/posix_use.output output

`PDI_multi_expose` shares `iter` and `field`, emits `newiter` while they are shared, then reclaims them.

Test the plugin from at least C and Fortran.
The Fortran version of this program, `plugins/posix/tests/posix_use.f90`, uses the same specification tree, with a transaction in place of `PDI_multi_expose`.
