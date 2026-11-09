# Set_value configuration reference {#set_value_reference}

This page documents the configuration of the \ref set_value_plugin "Set_value plugin", one section per key, named after its path as in the
\ref Specification_tree_ref "specification tree reference".
\ref set_value_examples "Set_value examples" shows how to combine them.

## `@set_value` {#set_value_config}

A mapping with the keys below.
Use it, as the value of `plugins.set_value` (see \ref spectree_pdi_plugins_plugin_name), to give the operations to execute, and when.

**Example:**

\snippet{trimleft} set_value/docs/reference.yml set_value_config


### `@set_value.on_init` {#set_value_on_init}

*Optional.*
A \ref set_value_operations.
Use `on_init` to execute the operations in \ref PDI_init, once all data and metadata are declared.

**Example:**

\snippet{trimleft} set_value/docs/reference.yml set_value_on_init


### `@set_value.on_event.<event_name>` {#set_value_on_event_event_name}

A \ref set_value_operations.
Use `on_event.<event_name>` to execute the operations each time the event named `<event_name>` is emitted.

**Example:**

\snippet{trimleft} set_value/docs/reference.yml set_value_on_event_event_name


### `@set_value.on_data.<data_name>` {#set_value_on_data_data_name}

A \ref set_value_operations.
Use `on_data.<data_name>` to execute the operations each time the data or metadata named `<data_name>` is shared.

**Example:**

\snippet{trimleft} set_value/docs/reference.yml set_value_on_data_data_name


### `@set_value.on_finalize` {#set_value_on_finalize}

*Optional.*
A \ref set_value_operations.
Use `on_finalize` to execute the operations in \ref PDI_finalize.

**Example:**

\snippet{trimleft} set_value/docs/reference.yml set_value_on_finalize


### `@set_value.logging` {#set_value_logging}

*Optional.*
A \ref spectree_logging.
Use `logging` to configure the logger of the plugin, see \ref spectree_pdi_plugins_plugin_name_logging.

**Example:**

\snippet{trimleft} set_value/docs/reference.yml set_value_logging


## `@operations` {#set_value_operations}

An *operation list*: a **sequence**, even of a single operation, whose elements are executed in order.
Each element is a mapping with a single key: the kind of operation, documented below.
You can use it in \ref set_value_on_init, \ref set_value_on_event_event_name,
\ref set_value_on_data_data_name and \ref set_value_on_finalize.

**Example:**

\snippet{trimleft} set_value/docs/reference.yml set_value_operations


### `@operations[*].set` {#set_value_operations_item_set}

A \ref set_value_value_list.
Use `set` to set the values of data that are already shared, with write access.

**Example:**

\snippet{trimleft} set_value/docs/reference.yml set_value_operations_item_set


### `@operations[*].share` {#set_value_operations_item_share}

A \ref set_value_value_list.
Use `share` to share the values, with read and write access.

\warning
The plugin allocates the memory itself, and frees it when the data is released.
If you reclaim the data instead, %PDI no longer manages that memory.

**Example:**

\snippet{trimleft} set_value/docs/reference.yml set_value_operations_item_share


### `@operations[*].expose` {#set_value_operations_item_expose}

A \ref set_value_value_list.
Use `expose` to expose the values: each data is shared, then released immediately.

**Example:**

\snippet{trimleft} set_value/docs/reference.yml set_value_operations_item_expose


### `@operations[*].release` {#set_value_operations_item_release}

A sequence of scalars.
Use `release` to release the data it names.

**Example:**

\snippet{trimleft} set_value/docs/reference.yml set_value_operations_item_release


### `@operations[*].event` {#set_value_operations_item_event}

A scalar.
Use `event` to emit the event it names.

**Example:**

\snippet{trimleft} set_value/docs/reference.yml set_value_operations_item_event


### `@operations[*].logger.level` {#set_value_operations_item_logger_level}

*Optional.*
A \ref spectree_expression, evaluated each time the operation is executed.
Use `logger.level` to change the level of the logger of %PDI to one of those of \ref spectree_logging_level.

**Example:**

\snippet{trimleft} set_value/docs/reference.yml set_value_operations_item_logger_level


### `@operations[*].logger.pattern` {#set_value_operations_item_logger_pattern}

*Optional.*
A scalar.
Use `logger.pattern` to change the pattern of the logger of %PDI and of those of the plugins that do not set their own.

**Example:**

\snippet{trimleft} set_value/docs/reference.yml set_value_operations_item_logger_pattern


### `@operations[*].logger.evaluate` {#set_value_operations_item_logger_evaluate}

*Optional.*
A \ref spectree_expression, evaluated once, at initialization.
Use `logger.evaluate` to evaluate the logger pattern again each time the operation is executed, if it is true, so that the pattern shows the
current values of the data it references.

**Example:**

\snippet{trimleft} set_value/docs/reference.yml set_value_operations_item_logger_evaluate


## `@value_list` {#set_value_value_list}

A *value list*: a sequence of single-key mappings, handled in order.
You can use it in \ref set_value_operations_item_set, \ref set_value_operations_item_share and \ref set_value_operations_item_expose.

**Example:**

\snippet{trimleft} set_value/docs/reference.yml set_value_value_list


### `@value_list[*].<data_name>` {#set_value_value_list_item_data_name}

A \ref spectree_expression, or a sequence or mapping of them, converted to the type of the data named `<data_name>`.
Use `<data_name>` to give the value of the data named `<data_name>`, either:
* a scalar, to set a scalar data,
* a sequence of values, to set an array,
* a mapping associating each member name with its value, to set a record.

**Example:**

\snippet{trimleft} set_value/docs/reference.yml set_value_value_list_item_data_name_scalar


**Example:**

\snippet{trimleft} set_value/docs/reference.yml set_value_value_list_item_data_name_array


**Example:**

\snippet{trimleft} set_value/docs/reference.yml set_value_value_list_item_data_name_record
