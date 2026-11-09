# User-code configuration reference {#user_code_reference}

This page documents the configuration of the \ref user_code_plugin "User-code plugin", one section per key, named after its path as in the
\ref Specification_tree_ref "specification tree reference".
\ref user_code_examples "User-code examples" shows how to combine them.

## `@user_code` {#user_code_config}

A mapping.
Use it, as the value of `plugins.user_code` (see \ref spectree_pdi_plugins_plugin_name), to give the functions to call, and when.
Keys not listed here are ignored.

**Example:**

\snippet{trimleft} user_code/docs/reference.yml user_code_config


### `@user_code.on_event` {#user_code_on_event}

*Optional.*
A mapping.
Use `on_event` to call functions when events are emitted.
It can also be a sequence of single-key mappings, which has the same meaning and lets an event appear more than once.

**Example:**

\snippet{trimleft} user_code/docs/reference.yml user_code_on_event


### `@user_code.on_event.<event_name>` {#user_code_on_event_event_name}

Either a single \ref user_code_function_list or a sequence of them.
Use `on_event.<event_name>` to call the functions each time the event named `<event_name>` is emitted.

**Example:**

\snippet{trimleft} user_code/docs/reference.yml user_code_on_event_event_name

`fun2` is called each time `my_event` is emitted, `fun1` only if, in addition, `$cond > 1`.


### `@user_code.on_data` {#user_code_on_data}

*Optional.*
A mapping.
Use `on_data` to call functions when data are shared.
It can also be a sequence of single-key mappings, which has the same meaning and lets a data appear more than once.

**Example:**

\snippet{trimleft} user_code/docs/reference.yml user_code_on_data


### `@user_code.on_data.<data_name>` {#user_code_on_data_data_name}

Either a single \ref user_code_function_list or a sequence of them.
Use `on_data.<data_name>` to call the functions each time the data named `<data_name>` is shared.

**Example:**

\snippet{trimleft} user_code/docs/reference.yml user_code_on_data_data_name


### `@user_code.logging` {#user_code_logging}

*Optional.*
A \ref spectree_logging.
Use `logging` to configure the logger of the plugin, see \ref spectree_pdi_plugins_plugin_name_logging.

**Example:**

\snippet{trimleft} user_code/docs/reference.yml user_code_logging


## `@function_list` {#user_code_function_list}

A *function list*: a mapping with the keys below, giving the functions to call, in order, and an optional condition.
You can use it in \ref user_code_on_event_event_name and \ref user_code_on_data_data_name.

**Example:**

\snippet{trimleft} user_code/docs/reference.yml user_code_function_list


### `@function_list.when` {#user_code_function_list_when}

*Optional.*
A \ref spectree_expression.
Use `when` to call the functions of the list only under a condition.
It is evaluated for each function, once its aliases are exposed, and the function is only called if it is true.
An error while evaluating it is logged and does not interrupt the program.

**Example:**

\snippet{trimleft} user_code/docs/reference.yml user_code_function_list_when


### `@function_list.<function_name>` {#user_code_function_list_function_name}

A mapping, empty (`{}`) when the function needs no alias.
Use `<function_name>` to call the function named `<function_name>`, with the aliases to expose while it runs.
The function must be of type `void(void)` and its symbol exported, see \ref dependencies_node.
A function cannot be named `when`: this key is reserved for the condition of the list.

**Example:**

\snippet{trimleft} user_code/docs/reference.yml user_code_function_list_function_name


### `@function_list.<function_name>.<alias_name>` {#user_code_function_list_function_name_alias_name}

A \ref spectree_expression.
Use `<alias_name>` to share its value under the name `<alias_name>` while the function runs.

**Example:**

\snippet{trimleft} user_code/docs/reference.yml user_code_function_list_function_name_alias_name
