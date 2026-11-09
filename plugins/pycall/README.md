# Pycall plugin {#pycall_plugin}

The Pycall plugin calls Python code written in the YAML file, when some data is shared or when the program emits some event.

## Configuration {#pycall_configuration}

The structure of its configuration:
\snippet{trimleft} pycall/docs/reference.yml pycall_configuration

## Examples {#pycall_ex}

A full example of the Pycall plugin, with its specification tree and the C code that uses it.

Specification tree:
\snippet{trimleft} pycall/docs/reference.yml pycall_ex

C code:
\snippet pycall/docs/pycall_example.c pycall_ex
Output of the program:
\snippet pycall/docs/pycall_example.output output

## `@pycall` {#pycall_config}

A mapping.
Use it, as the value of `plugins.pycall` (see \ref spectree_pdi_plugins_plugin_name), to give the Python code to execute, and when.

The code is executed in the scope of the `__main__` Python module, where the `pdi` Python module is also available under the name `pdi`.
Variables defined by the code, and values exposed to it, remain in that scope and are visible to the code executed afterwards.
An exception raised by the Python code is logged as an error; it does not interrupt the program.

**Example:**

\snippet{trimleft} pycall/docs/reference.yml pycall_config


### `@pycall.on_event.<event_name>` {#pycall_on_event_event_name}

Either a single \ref pycall_pycode or a sequence of them.
Use `on_event.<event_name>` to execute the code, in order, each time the event named `<event_name>` is emitted.

**Example:**

\snippet{trimleft} pycall/docs/reference.yml pycall_on_event_event_name

The code sees the value of `a` as `a_python`, and can change it if `a` is shared with write access.


**Example:**

\snippet{trimleft} pycall/docs/reference.yml pycall_on_event_event_name_sequence


### `@pycall.on_data.<data_name>` {#pycall_on_data_data_name}

A scalar.
Use `on_data.<data_name>` to execute this Python code each time the data named `<data_name>` is shared.
The data is exposed to the code as the variable named `<data_name>`.

**Example:**

\snippet{trimleft} pycall/docs/reference.yml pycall_on_data_data_name


### `@pycall.logging` {#pycall_logging}

*Optional.*
A \ref spectree_logging.
Use `logging` to configure the logger of the plugin, see \ref spectree_pdi_plugins_plugin_name_logging.

**Example:**

\snippet{trimleft} pycall/docs/reference.yml pycall_logging


## `@pycode` {#pycall_pycode}

A piece of Python code to execute, and the values it sees: a mapping with the keys below.
You can use it in \ref pycall_on_event_event_name.

**Example:**

\snippet{trimleft} pycall/docs/reference.yml pycall_pycode


### `@pycode.exec` {#pycall_pycode_exec}

*Required.*
A scalar.
Use `exec` to give the Python code to execute.

**Example:**

\snippet{trimleft} pycall/docs/reference.yml pycall_pycode_exec


### `@pycode.with.<python_name>` {#pycall_pycode_with_python_name}

*Optional.*
A \ref spectree_expression.
Use `with.<python_name>` to expose its value to the Python code as the variable named `<python_name>`.
`with` can be omitted: no value is then exposed for this execution of the code.

**Example:**

\snippet{trimleft} pycall/docs/reference.yml pycall_pycode_with_python_name


