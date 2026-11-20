# Plugin design guidelines {#plugin_design_guidelines}

This page is for developers who know a library well and want to make it available to %PDI users through a plugin.
It does not explain how to write the code of a plugin, \ref how_to_create_a_plugin "the plugin creation tutorial" does.
It explains what a plugin should look like *from the outside*, so that it fits the declarative approach %PDI is built on.

%PDI decouples a simulation code from the libraries that handle its data.
The application is instrumented once, in a library-agnostic way:
it *shares* the buffers it produces or consumes, and it *emits events* at the noteworthy points of its execution.
It describes what it has and where it is; it never says what to do with it.
The \ref Specification_tree_ref "specification tree", a YAML file, then says which plugins act on that data, when, and how.
Users can thus change what happens to their data (write it to HDF5 instead of NetCDF, checkpoint it, send it to an in situ analysis) by editing the
YAML file alone, and the same instrumentation serves C, Fortran and Python codes alike.

A plugin is the translator between this model and a library.
Its job is to let a user who knows the library express, in the specification tree, what they would otherwise have written as calls in the
application, with the data and events the application already exposes.
This turns the usual library design upside down: the library does not get to require anything from the application, every decision goes into the
configuration, and the plugin has to cope with whatever the application chose to expose.

The \ref Decl_HDF5_plugin "Decl'HDF5 plugin" is the plugin whose design comes closest to what these guidelines describe, and the examples below
draw on it.
When in doubt, look at how it handles a similar question.

[TOC]

## The application stays plugin-agnostic {#pdg_application}

### The application describes, the configuration decides {#pdg_config_decides}

**Every decision about what the plugin does belongs in the specification tree.**
Whether to write or read, whether to recover from a checkpoint, which step to output: none of this should be decided by the application, nor
inferred by the plugin from what the application did.
Where the decision depends on the state of the simulation, let the configuration express it with a $-expression, as Decl'HDF5 does with `when`:

```yaml
plugins:
  decl_hdf5:
    file: output.h5
    on_event: newiter
    when: '$iter % 10 = 0'   # the simulation state drives the decision, through the configuration
    write: [main_field]
```

A recovery decision, for example, can take the form of a `recover_when:` key evaluated like `when`, or rely on what the library knows (whether a
checkpoint exists).

**The access permissions of a share must not select a behaviour.**
A buffer shared with `PDI_OUT` is one the plugin may read, one shared with `PDI_IN` one the plugin may write, but the plugin must not infer from
that what it is asked to do.
The application picks a direction for its own reasons, and another plugin may need it to share a buffer `PDI_INOUT`: adding that plugin must not
change what yours does.
The behaviour is selected by the configuration; the plugin then asks for the permission that behaviour needs, and reports an error when the
share does not grant it.
In particular, a plugin that writes when it can read and reads when it can write does both, in that order, on a `PDI_INOUT` share, overwriting the
value the application set before anyone reads it.

A plugin that only relays a share is not selecting a behaviour.
For example, it can share a converted copy of the buffer with the same permissions, filling it from the original if the share lets it read and
copying it back if the share lets it write.
The data then behaves exactly as if the application had shared it directly.
Such a plugin must keep that promise in every direction: a `PDI_IN` copy that no plugin wrote must not overwrite the application's buffer when it
is reclaimed.

**Support both ways of triggering an action.**
Decl'HDF5 executes an operation either whenever the data it names is shared, or on an event given by `on_event`.
Neither the names of these data nor the names of these events are chosen by the plugin: they come from the configuration.

### The application needs no plugin-specific code {#pdg_no_specific_code}

**The application should not have to change for a plugin to be used.**
It shares the data it has and emits events at meaningful points in its execution; nothing in it should be there for one plugin in particular.
Concretely, a plugin should not require:

* data that only it uses, such as a status flag the application never looks at;
* a sequence of events with a plugin-specific meaning (`begin`, `route`, `end`), when an event the application already emits would do;
* the application to read settings of the plugin from its own configuration, outside the `.pdi` subtree.

Most uses of %PDI handle cross-cutting concerns: output, checkpointing, monitoring, analysis.
The application must keep running, correctly, with your plugin removed from the configuration.

The examples shipped with %PDI (`example/example.c` and its Fortran and Python twins) follow the same rule: they do the exact same thing in the
three languages and share no data or event specific to one plugin.
Demonstrate a plugin through a YAML file next to them, or in its own tests, not by changing the application.

### Plugins are language-agnostic {#pdg_language_agnostic}

**Do not assume the language of the application.**
A buffer is described by its %PDI type, not by a C declaration.
Reinterpreting a shared buffer as a C-only type breaks for the other languages:
a Fortran code, for example, shares an MPI communicator as an integer (`MPI_Fint`), not as a C `MPI_Comm`.
Take such handles from a $-expression instead, and let the \ref mpi_plugin "MPI plugin" provide them correctly in every language
(`communicator: $MPI_COMM_WORLD`).
Test the plugin from at least C and Fortran.

## The specification tree {#pdg_specification_tree}

### Mirror the library's concepts {#pdg_library_concepts}

**Model the configuration on the objects of the library, not on the calls it takes to use them.**
The user of a plugin usually knows the library: they should recognize its concepts in the configuration and find the setting they are looking for
where they expect it.
Decl'HDF5 describes files, datasets, attributes and selections, with the actions to perform on them; the calls that open, create, select and close
are the plugin's business.
Name the keys after the library's own vocabulary (`deflate`, `fletcher`, `chunking` and `mpio` in Decl'HDF5), and link to the library's
documentation of each of them rather than paraphrasing it.

Exposing a subset of the library is fine, as long as the subset is consistent and documented.
Do not invent concepts the library does not have to make up for a missing feature of %PDI: propose that feature to %PDI instead.

### Simple things are simple {#pdg_defaults}

**Give every key a sensible default, and offer short forms for the common cases.**
In Decl'HDF5, a list of data names is enough to write each of them to a dataset of the same name;
the full form, a map of `DATA_IO_DESC`s, is only needed for something more specific.
Settings given at an outer level (`when`, `communicator` or `deflate` for a whole file) serve as defaults for the inner ones, which can override
them.
The configuration for the simple case should be short enough to be guessed, and the full configuration still available when needed.

### Plugin inputs are $-expressions {#pdg_inputs}

**Every value the plugin takes from the configuration should be a $-expression.**
The rest of %PDI does so (`communicator: $MPI_COMM_WORLD`, `when: '$iter<10'`, `file: data${rank}.h5`), and it lets users adapt and combine the
values the simulation exposes, without modifying it.
A plain string naming a data that the plugin fetches and casts is much more restrictive:

* an iteration number given as the name of a data must be exposed with exactly the type the plugin casts it to, while `$iter` accepts any
  integer, and `$iter + $restart_offset` a computed value;
* a file name given as a literal cannot depend on the rank or the step, unlike `file: data${rank}.h5`.

Plain names are still the right choice for the data the plugin acts *on*, such as the data Decl'HDF5 writes or reads: those are not values, but
designations of buffers in the data store.

### Plugin outputs go into user-chosen buffers {#pdg_outputs}

**A plugin that produces a value writes it into a buffer the user names in the configuration.**
The application allocates and shares the buffer; the plugin only fills it.
The user can then place the result where the code needs it, without changing the simulation and without an extra copy.
This is how Decl'HDF5 reads a dataset: into the data named in its `read` section, and how `size_of` returns the size of a dataset.
A plugin that claims fixed names for its outputs (`status`, `counter`) forces the application to adopt them, and collides with any other use of
those names.

**Check the type of the buffer before writing to it.**
The plugin knows the %PDI type of every buffer it is given: it must check that this type can hold what it is about to write, and report an error
otherwise.
A size limit stated in the documentation is only a convention, and a smaller buffer then silently overflows.

### Validate at load time, evaluate at trigger time {#pdg_validate}

**Parse the whole configuration in the plugin's constructor, and reject what you do not understand.**
An unknown key is far more often a typo than a deliberate choice: report it as an error, with the location in the specification tree, rather
than ignore it.
The user then finds configuration mistakes at start-up, rather than hours into a run when the faulty operation first triggers.
The `logging` key is the one key every plugin accepts without handling it itself: %PDI configures the plugin's logger from it.

**Evaluate $-expressions only when the action triggers.**
The values they reference change during the run, and may not even be shared yet when the plugin is loaded.
Decl'HDF5 keeps every `file`, `when` and `communicator` as an expression and evaluates it each time an operation executes.

## Interacting with the data store {#pdg_data_store}

### Access data only through references {#pdg_references}

**Hold a reference for as long as anybody, the library included, accesses the data.**
References hold the read/write locks that give a plugin exclusive access to a buffer it writes, and keep the buffer alive.
A raw pointer obtained from a reference is only valid while the reference is held.
When a library accesses the buffers asynchronously, or between two calls (one that registers a buffer, one that later reads or writes it), the
plugin must keep the references (a container of `Ref_r` or `Ref_w`) until the library no longer uses the buffers.
A reference that is a local variable inside a loop is released at the end of each iteration, before the library uses the buffer, and other
plugins can then access it concurrently.

Conversely, release a reference as soon as the access is over, so that the application and the other plugins regain access to the data.

### Use the %PDI type of the data {#pdg_types}

**Build the library's description of the data from its %PDI type.**
The type of a buffer is part of what the application exposes: its sizes, its layout in memory (arrays with a start and a subsize, records,
pointers) and its scalar kind.
Decl'HDF5 derives the HDF5 type and memory layout of each data from it, so that any data with an HDF5 equivalent can be written as is, without the
application having to copy it into a dense buffer first.
Support every type the library can represent, and report a clear error, naming the data and its type, for those it cannot.

### Plugins are composable {#pdg_composable}

**Design a plugin to work alongside the others, not in their place.**
A plugin should expect nothing from the application beyond the data it produces and the buffers it lets plugins fill.
What it produces can then be consumed by another plugin, and what it consumes produced by another one.
The \ref serialize_plugin "Serialize plugin", for example, takes a data the user selects and shares its serialized form under a name the user
selects too, for any other plugin to use.

Prefer making your plugin work with the existing plugins to duplicating what they do.
A checkpointing library that hands back a path to write the checkpoint to should not ask the application to write the file itself: it should share
that path as data, so that Decl'HDF5 or Decl'NetCDF write the file with `file: ${checkpoint_file}`, the plugin framing their writes with the
library's begin and end calls on events the user selects.

## The library, errors and logging {#pdg_library}

### The plugin owns the library's life cycle {#pdg_life_cycle}

**Initialize the library in the plugin, and finalize it there.**
The application should not have to initialize a library it does not call itself.
Decl'HDF5 initializes HDF5 in its constructor and finalizes it in its destructor.
When the initialization needs a value from the application, such as a communicator, take it from a $-expression and initialize the library when
it first becomes available, at the latest on its first use.
Conversely, do not finalize a library the application uses on its own, such as MPI.

### Errors are reported, never hidden {#pdg_errors}

**Turn every error the library reports into an exception, with the library's own message.**
Check every return code, and throw a `PDI::Error` that says what the plugin was doing, for which data and with which file or object, along with
the message of the library.
%PDI then reports it to the application according to its error handling settings.
Never call `exit` or `abort`, and never let a library error go unnoticed.

**Log through the logger the plugin is given**, at the levels the
[contribution guide](https://github.com/pdidev/pdi/blob/main/CONTRIBUTING.md#logging) defines:
`info` for the few steps that help the user understand what happened, `debug` for what helps them diagnose a configuration that does not
behave as expected.

## Documentation and tests {#pdg_documentation}

**Document the configuration grammar in full, with an example.**
The plugin's `README.md` is its user documentation: describe every key, its type (value, $-expression, data name, event name), its default value,
and the library concept it maps to, as the Decl'HDF5 documentation does, and end with a complete commented example.

**Test every key, from more than one language.**
Each feature of the configuration should be exercised by a test, including the errors it reports for an invalid configuration or an incompatible
buffer.
