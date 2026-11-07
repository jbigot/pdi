# The user-code plugin {#user_code_plugin}

The `user-code` plugin calls user-defined functions when a given event is
emitted, or when some data becomes available.

This page is the reference of its configuration, one section per key.
\subpage user_code_examples "User-code examples" shows, one approach at a time, how to combine them with your code.

## Important notes {#important_notes_node}

* Make sure to compile your program with the `-Wl,--export-dynamic` or `-rdynamic`
  flag (in CMake, set the `ENABLE_EXPORTS` property to `TRUE` with the
  `set_target_properties` command), so that its symbols are exported.
* Make sure to use the proper access rights in the calls to \ref PDI_access of your
  functions (`PDI_IN` for reading, `PDI_OUT` for writing, `PDI_INOUT` for both).
* Aliases make it possible to pass different data to a function without
  recompiling the code.

## Dependencies between the code and the specification tree {#dependencies_node}

For the `user-code` plugin to work properly, the application code and the specification tree must follow
several conventions.

First, each function name in the specification tree must be the name of a symbol of the program (watch out for [name mangling](https://en.wikipedia.org/wiki/Name_mangling)). A function cannot be named `when`: this key is reserved for the condition of a function list.

Second, these functions **cannot** take any argument or return any value (*i.e.* their type must be `void(void)`). Use aliases to pass input and output variables instead.

Third, the symbols of the functions **must** be exported, for the `user-code` plugin to find them. To do so, compile your program as described in \ref important_notes_node.

## `@user_code` {#user_code_config}

A mapping.
Use it, as the value of `plugins.user_code` (see \ref spectree_pdi_plugins_plugin_name), to give the functions to call, and when.
Keys not listed here are ignored.

**Example:**

```yaml
plugins:
  user_code:
    on_event:
      checkpoint:         # <- functions called on an event...
        write_checkpoint: {}
    on_data:
      field:              # <- ... and when data is shared
        process_field: {input: $field}
```


### `@user_code.on_event` {#user_code_on_event}

*Optional.*
A mapping.
Use `on_event` to call functions when events are emitted.
It can also be a sequence of single-key mappings, which has the same meaning and lets an event appear more than once.

**Example:**

```yaml
on_event:                 # <- functions called on events
  checkpoint:
    write_checkpoint: {}
```


### `@user_code.on_event.<event_name>` {#user_code_on_event_event_name}

Either a single \ref user_code_function_list or a sequence of them.
Use `on_event.<event_name>` to call the functions each time the event named `<event_name>` is emitted.

**Example:**

```yaml
on_event:
  - my_event:             # <- two function lists for the same event, in the sequence form
      when: $cond > 1
      fun1: {in: $desc2, out: $desc3}
  - my_event:
      fun2: {}
```

`fun2` is called each time `my_event` is emitted, `fun1` only if, in addition, `$cond > 1`.


### `@user_code.on_data` {#user_code_on_data}

*Optional.*
A mapping.
Use `on_data` to call functions when data are shared.
It can also be a sequence of single-key mappings, which has the same meaning and lets a data appear more than once.

**Example:**

```yaml
on_data:                  # <- functions called when data is shared
  field:
    process_field: {input: $field}
```


### `@user_code.on_data.<data_name>` {#user_code_on_data_data_name}

Either a single \ref user_code_function_list or a sequence of them.
Use `on_data.<data_name>` to call the functions each time the data named `<data_name>` is shared.

**Example:**

```yaml
on_data:
  field:                  # <- called each time field is shared
    process_field: {input: $field}
    report: {}
```


### `@user_code.logging` {#user_code_logging}

*Optional.*
A \ref spectree_logging.
Use `logging` to configure the logger of the plugin, see \ref spectree_pdi_plugins_plugin_name_logging.

**Example:**

```yaml
logging: debug            # <- the logger of the User-code plugin
on_event:
  checkpoint:
    write_checkpoint: {}
```


## `@function_list` {#user_code_function_list}

A *function list*: a mapping with the keys below, giving the functions to call, in order, and an optional condition.
You can use it in \ref user_code_on_event_event_name and \ref user_code_on_data_data_name.

**Example:**

```yaml
on_event:
  step:                   # <- this mapping is a function list
    when: $iter > 0
    compute: {in: $field, out: $result}
    report: {}
```


### `@function_list.when` {#user_code_function_list_when}

*Optional.*
A \ref spectree_expression.
Use `when` to call the functions of the list only under a condition.
It is evaluated for each function, once its aliases are exposed, and the function is only called if it is true.
An error while evaluating it is logged and does not interrupt the program.

**Example:**

```yaml
when: $iter % 10 = 0      # <- the functions are only called every 10 iterations
report: {}
```


### `@function_list.<function_name>` {#user_code_function_list_function_name}

A mapping, empty (`{}`) when the function needs no alias.
Use `<function_name>` to call the function named `<function_name>`, with the aliases to expose while it runs.
The function must be of type `void(void)` and its symbol exported, see \ref dependencies_node.
A function cannot be named `when`: this key is reserved for the condition of the list.

**Example:**

```yaml
compute: {in: $field}     # <- calls compute(), with field shared as in while it runs
report: {}                # <- calls report(), with nothing shared
```


### `@function_list.<function_name>.<alias_name>` {#user_code_function_list_function_name_alias_name}

A \ref spectree_expression.
Use `<alias_name>` to share its value under the name `<alias_name>` while the function runs.

**Example:**

```yaml
compute:
  in: $field              # <- compute() accesses field as in
  out: $result            #    and result as out
```
