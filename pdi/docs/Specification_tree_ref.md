# Specification tree Reference {#Specification_tree_ref}

The %PDI specification tree is expressed in \subpage YAML.
This reference documents each key of the tree in its own section, named after the path leading to it from a block.

The paths follow these conventions:
* a dot `.` separates a key from the key that contains it, *e.g.* `@pdi.plugins.mpi` is the `mpi` key of the `plugins` mapping of the `@pdi` block,
* a name between angle brackets is a key whose name you choose, *e.g.* `@pdi.data.<data_name>` is any key of the `data` mapping,
* `[*]` stands for any element of a sequence, *e.g.* `@struct.members[*]`,
* a name preceded by `@` is a *block*: a part of the tree documented once, in its own section, to which each key that takes it as value links,
  *e.g.* `@logging` is the configuration of a logger, the value of both \ref spectree_pdi_logging and \ref spectree_pdi_plugins_plugin_name_logging, and
  `@logging.level` is its `level` key,
* names between parentheses and separated by `|` are alternatives, *e.g.* `(@character|@integer|@logical|@real).kind` is the `kind` key of
  any of these four datatypes.

Datatypes and $-expressions can appear in many places in the tree, including in the configuration of plugins.
Each is documented once, on its own page: \subpage Spectree_types_ref "Datatypes" and \subpage spectree_expression "$-expressions".
Each plugin documents its own blocks on its own page.

The root of the specification tree is a \ref spectree_pdi block.
It is either the root of the YAML document, or a subtree of it selected when initializing %PDI (see
\ref parse_subtree "Parsing a subtree").


## `@pdi` {#spectree_pdi}

A mapping with the keys below: the configuration of %PDI.
Keys not listed below are ignored.

**Example:**

\snippet{trimleft} Specification_tree_ref/spectree.yml spectree_pdi

This is a whole specification tree: either the root of a YAML file passed to `PDI_init`, or a subtree of one.


### `@pdi.include` {#spectree_pdi_include}

*Optional.*
Either a single \ref spectree_include or a sequence of them.
Use `include` to read parts of the specification tree from other YAML files, or from subtrees of files.

An included tree has the same format as the root of the specification tree.
Its `include`, `types`, `metadata`, `data`, `plugin_path` and `plugins` keys are taken into account, while its \ref spectree_pdi_logging key is
ignored.
The keys under these sections must not be repeated: a type, a data, a metadata or a plugin defined in two files is an error.

The same file can be included twice (a diamond include), in which case it is only read once, but a recursive inclusion is an error.

**Example:**

\snippet{trimleft} Specification_tree_ref/spectree.yml spectree_pdi_include

The scalar form is a shortcut for `include: [{file: common.yml, subtree: ""}]`.


### `@pdi.logging` {#spectree_pdi_logging}

*Optional.*
A \ref spectree_logging.
Use `logging` to configure the logger of %PDI.
It is only read from the root file, not from included ones.
Each plugin has its own logger, see \ref spectree_pdi_plugins_plugin_name_logging.

**Example:**

\snippet{trimleft} Specification_tree_ref/spectree.yml spectree_pdi_logging


### `@pdi.types.<type_name>` {#spectree_pdi_types_type_name}

A \ref spectree_datatype.
Use `types.<type_name>` to define a datatype named `<type_name>`.
`<type_name>` can then be used as the value of \ref spectree_datatype_type anywhere a datatype is expected.

A user-defined datatype cannot reuse the name of an existing datatype.

**Example:**

\snippet{trimleft} Specification_tree_ref/spectree.yml spectree_pdi_types_type_name


### `@pdi.metadata.<data_name>` {#spectree_pdi_metadata_data_name}

A \ref spectree_datatype.
Use `metadata.<data_name>` to declare a metadata named `<data_name>`, of this type.
%PDI keeps a copy of the value of a metadata when it is shared, so that it can be referenced in $-expressions afterwards.

A name can only be defined once among all \ref spectree_pdi_metadata_data_name and \ref spectree_pdi_data_data_name keys.

**Example:**

\snippet{trimleft} Specification_tree_ref/spectree.yml spectree_pdi_metadata_data_name


### `@pdi.data.<data_name>` {#spectree_pdi_data_data_name}

A \ref spectree_datatype.
Use `data.<data_name>` to declare a data named `<data_name>`, of this type.
Unlike with a metadata, %PDI only keeps a reference to a data while it is shared.

A name can only be defined once among all \ref spectree_pdi_metadata_data_name and \ref spectree_pdi_data_data_name keys.

**Example:**

\snippet{trimleft} Specification_tree_ref/spectree.yml spectree_pdi_data_data_name


### `@pdi.plugin_path` {#spectree_pdi_plugin_path}

*Optional.*
Either a single scalar or a sequence of them.
Use `plugin_path` to list directories in which %PDI searches for plugins.

%PDI takes the first match, searching the directories of the `PDI_PLUGIN_PATH` environment variable first, then those of `plugin_path`, then the
directory where %PDI installs its plugins, and finally the system library path.

**Example:**

\snippet{trimleft} Specification_tree_ref/spectree.yml spectree_pdi_plugin_path


### `@pdi.plugins.<plugin_name>` {#spectree_pdi_plugins_plugin_name}

Use `plugins.<plugin_name>` to load the plugin named `<plugin_name>` and to configure it.
Its content depends on the plugin: see the \ref Plugins "plugins" documentation for the specification tree each one accepts.
It can be empty (`~`) for a plugin that needs no configuration.

A plugin can only be loaded once, even across included files.

**Example:**

\snippet{trimleft} Specification_tree_ref/spectree.yml spectree_pdi_plugins_plugin_name


### `@pdi.plugins.<plugin_name>.logging` {#spectree_pdi_plugins_plugin_name_logging}

*Optional.*
A \ref spectree_logging.
Use `logging` to configure the logger of the plugin.
This key is handled by %PDI for every plugin whose configuration is a mapping.

**Example:**

\snippet{trimleft} Specification_tree_ref/spectree.yml spectree_pdi_plugins_plugin_name_logging

Each plugin documents this key too, in its own reference.


## `@logging` {#spectree_logging}

The configuration of a logger, either:
* a scalar, a shortcut for a mapping whose only key is \ref spectree_logging_level with that value,
* a mapping with the keys below.
You can use it in \ref spectree_pdi_logging and \ref spectree_pdi_plugins_plugin_name_logging.

**Example:**

\snippet{trimleft} Specification_tree_ref/spectree.yml spectree_logging


### `@logging.level` {#spectree_logging_level}

*Optional.*
A scalar.
Use `level` to set the verbosity of the logger, to one of the following values, from the most to the least verbose:

|`level`  |what is logged                                                                                                              |
|:--------|:---------------------------------------------------------------------------------------------------------------------------|
|`trace`  |every step of the execution, to debug the library itself                                                                    |
|`debug`  |the normal situations of the execution that might help to understand the behavior of the library                            |
|`info`   |the normal situations of the execution that are likely to help understand the behavior of the library                       |
|`warn`   |the situations that the library detects as very likely invalid (user input that is technically valid, but very unusual)     |
|`error`  |the invalid situations that the library detects (invalid user input, invalid hardware behavior, etc.)                       |
|`off`    |nothing                                                                                                                     |

By default, the level is `info` for %PDI, and that of %PDI for a plugin.

**Example:**

\snippet{trimleft} Specification_tree_ref/spectree.yml spectree_logging_level


### `@logging.pattern` {#spectree_logging_pattern}

*Optional.*
A scalar.
Use `pattern` to set the format of a log line, as an spdlog pattern.

The pattern is parsed by the spdlog library (see https://github.com/gabime/spdlog/wiki/3.-Custom-formatting).
It specifies the format of the whole line, so it should contain `%v`, the logged message itself.

%PDI adds a special flag, `%{<REF>}`, where `<REF>` is the `REFERENCE` part of a \ref spectree_expression without its leading `$` (with an
optional format specification).
It is evaluated just after all plugins have been initialized.

By default, the pattern is (where `%n` is `PDI` or a plugin name):
\snippet Specification_tree_ref/default_patterns.txt serial
for a serial execution, and:
\snippet Specification_tree_ref/default_patterns.txt mpi
when the \ref mpi_plugin "MPI plugin" is loaded.
The pattern of a plugin defaults to that of %PDI.

**Example:**

\snippet{trimleft} Specification_tree_ref/spectree.yml spectree_logging_pattern

`MPI_COMM_WORLD_rank` is shared by the \ref mpi_plugin "MPI plugin", which must then be loaded.


### `@logging.output.file` {#spectree_logging_output_file}

*Optional.*
A scalar.
Use `output.file` to write the logs to the file at this path.

**Example:**

\snippet{trimleft} Specification_tree_ref/spectree.yml spectree_logging_output_file


### `@logging.output.console` {#spectree_logging_output_console}

*Optional.*
A scalar.
Use `output.console` to choose whether to write the logs to the standard output:

|`console`                         |the logs are                     |
|:---------------------------------|:--------------------------------|
|`off`                             |not written to the console       |
|`on`, or any other value          |written to the standard output   |

By default, it is `off` when \ref spectree_logging_output_file is defined and `on` otherwise.

**Example:**

\snippet{trimleft} Specification_tree_ref/spectree.yml spectree_logging_output_console


## `@include` {#spectree_include}

An *include directive*, either:
* a scalar: the path of a file whose root is included, a shortcut for a mapping whose `file` is that scalar and whose `subtree` is empty,
* a mapping with both an \ref spectree_include_file and an \ref spectree_include_subtree key, and no other key.
You can use it in \ref spectree_pdi_include.

**Example:**

\snippet{trimleft} Specification_tree_ref/spectree.yml spectree_include


### `@include.file` {#spectree_include_file}

*Required in a mapping include directive.*
A scalar.
Use `file` to give the path of the file to include.
The path is interpreted relative to the working directory of the execution.

**Example:**

\snippet{trimleft} Specification_tree_ref/spectree.yml spectree_include_file


### `@include.subtree` {#spectree_include_subtree}

*Required in a mapping include directive.*
A scalar.
Use `subtree` to include only part of \ref spectree_include_file, given as a ypath, an empty one standing for the root of the file.

The ypath follows the syntax of paraconf, where:
* a dot accesses the value of a mapping by its key, *e.g.* `.map.key`,
* square brackets access an element of a sequence by its 0-based index, *e.g.* `.seq[1]`,
* braces access a key of a mapping by its 0-based index, *e.g.* `.map{1}`,
* chevrons access a value of a mapping by its 0-based index, *e.g.* `.map<1>`.

**Example:**

\snippet{trimleft} Specification_tree_ref/spectree.yml spectree_include_subtree


