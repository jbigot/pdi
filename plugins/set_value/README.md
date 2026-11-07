# Set_value plugin {#set_value_plugin}

The Set_value plugin allows setting values to data and metadata descriptors from
the yaml file.

Here are main features of the plugins:

1. Share data - plugin will share new allocated data with given values
2. Release data - plugin will release shared data
3. Expose data - plugin will expose new allocated data with given values
4. Set data - plugin will set given values to the already shared data
5. Calling an event - plugin will call an event

## `@set_value` {#set_value_config}

A mapping with the keys below.
Use it, as the value of `plugins.set_value` (see \ref spectree_pdi_plugins_plugin_name), to give the operations to execute, and when.

**Example:**

```yaml
plugins:
  set_value:              # <- operations to execute in PDI_init and in PDI_finalize
    on_init:
      - share:
          - counter: 0
    on_finalize:
      - release: [counter]
```


### `@set_value.on_init` {#set_value_on_init}

*Optional.*
A \ref set_value_operations.
Use `on_init` to execute the operations in \ref PDI_init, once all data and metadata are declared.

**Example:**

```yaml
on_init:                  # <- run in PDI_init
  - share:
      - counter: 0
on_finalize:
  - release: [counter]
```


### `@set_value.on_event.<event_name>` {#set_value_on_event_event_name}

A \ref set_value_operations.
Use `on_event.<event_name>` to execute the operations each time the event named `<event_name>` is emitted.

**Example:**

```yaml
on_event:
  increment:              # <- run each time the increment event is emitted
    - set:
        - counter: $counter + 1
```


### `@set_value.on_data.<data_name>` {#set_value_on_data_data_name}

A \ref set_value_operations.
Use `on_data.<data_name>` to execute the operations each time the data or metadata named `<data_name>` is shared.

**Example:**

```yaml
on_data:
  field:                  # <- run each time field is shared
    - expose:
        - field_shared: 1
```


### `@set_value.on_finalize` {#set_value_on_finalize}

*Optional.*
A \ref set_value_operations.
Use `on_finalize` to execute the operations in \ref PDI_finalize.

**Example:**

```yaml
on_finalize:              # <- run in PDI_finalize
  - event: finalizing
```


### `@set_value.logging` {#set_value_logging}

*Optional.*
A \ref spectree_logging.
Use `logging` to configure the logger of the plugin, see \ref spectree_pdi_plugins_plugin_name_logging.

**Example:**

```yaml
logging: debug            # <- the logger of the Set-value plugin
on_init:
  - event: initialized
```


## `@operations` {#set_value_operations}

An *operation list*: a **sequence**, even of a single operation, whose elements are executed in order.
Each element is a mapping with a single key: the kind of operation, documented below.
You can use it in \ref set_value_on_init, \ref set_value_on_event_event_name,
\ref set_value_on_data_data_name and \ref set_value_on_finalize.

**Example:**

```yaml
on_event:
  step:                   # <- this sequence is an operation list, run in order
    - set:
        - counter: $counter + 1
    - event: counter_incremented
```


### `@operations[*].set` {#set_value_operations_item_set}

A \ref set_value_value_list.
Use `set` to set the values of data that are already shared, with write access.

**Example:**

```yaml
- set:                    # <- counter is already shared, with write access
    - counter: $counter + 1
```


### `@operations[*].share` {#set_value_operations_item_share}

A \ref set_value_value_list.
Use `share` to share the values, with read and write access.

\warning
The plugin allocates the memory itself, and frees it when the data is released.
If you reclaim the data instead, %PDI no longer manages that memory.

**Example:**

```yaml
- share:                  # <- allocates buffer and shares it, until it is released
    - buffer: [0, 0, 0]
```


### `@operations[*].expose` {#set_value_operations_item_expose}

A \ref set_value_value_list.
Use `expose` to expose the values: each data is shared, then released immediately.

**Example:**

```yaml
- expose:                 # <- shares step, then releases it
    - step: 1
```


### `@operations[*].release` {#set_value_operations_item_release}

A sequence of scalars.
Use `release` to release the data it names.

**Example:**

```yaml
- release: [buffer]       # <- releases buffer, shared by an earlier share operation
```


### `@operations[*].event` {#set_value_operations_item_event}

A scalar.
Use `event` to emit the event it names.

**Example:**

```yaml
- event: checkpoint       # <- emits checkpoint, for the other plugins
```


### `@operations[*].logger.level` {#set_value_operations_item_logger_level}

*Optional.*
A \ref spectree_expression, evaluated each time the operation is executed.
Use `logger.level` to change the level of the logger of %PDI to one of those of \ref spectree_logging_level.

**Example:**

```yaml
- logger:
    level: debug          # <- PDI logs in detail from now on
```


### `@operations[*].logger.pattern` {#set_value_operations_item_logger_pattern}

*Optional.*
A scalar.
Use `logger.pattern` to change the pattern of the logger of %PDI and of those of the plugins that do not set their own.

**Example:**

```yaml
- logger:
    pattern: "[%n] %v"    # <- a shorter pattern from now on
```


### `@operations[*].logger.evaluate` {#set_value_operations_item_logger_evaluate}

*Optional.*
A \ref spectree_expression, evaluated once, at initialization.
Use `logger.evaluate` to evaluate the logger pattern again each time the operation is executed, if it is true, so that the pattern shows the
current values of the data it references.

**Example:**

```yaml
- logger:
    pattern: "[iteration %{iter}][%n] %v"
    evaluate: 1           # <- the iteration in the pattern is updated each time this operation runs
```


## `@value_list` {#set_value_value_list}

A *value list*: a sequence of single-key mappings, handled in order.
You can use it in \ref set_value_operations_item_set, \ref set_value_operations_item_share and \ref set_value_operations_item_expose.

**Example:**

```yaml
- share:
    - step: 0             # <- this sequence is a value list, handled in order
    - position: [0.0, 0.0, 0.0]
```


### `@value_list[*].<data_name>` {#set_value_value_list_item_data_name}

A \ref spectree_expression, or a sequence or mapping of them, converted to the type of the data named `<data_name>`.
Use `<data_name>` to give the value of the data named `<data_name>`, either:
* a scalar, to set a scalar data,
* a sequence of values, to set an array,
* a mapping associating each member name with its value, to set a record.

**Example:**

```yaml
- step: 42                # <- a scalar
```


**Example:**

```yaml
- position: [1.0, 2.0, 3.0]   # <- an array
```


**Example:**

```yaml
- particle:               # <- a record, member by member
    id: 3
    position: [1.0, 2.0, 3.0]
```


## full configuration example {#full_config}

```yaml
metadata:
  array_size: int64

data:
  record_data:
    type: record
    buffersize: 36
    members:
      scalar_data: 
        disp: 0
        type: int
      array_data:
        disp: 4
        type: array
        size: $array_size
        subtype: int

plugins:
  set_value:
    on_init:
      - expose:
        - array_size: 3
      - share:
        - record_data:
            scalar_data: 0
            array_data: [0, 0, 0]
    on_event:
      event_1_name:
        - set:
          - record_data:
              scalar_data: 3
              array_data: [1, 2, 3]
    on_finalize:
      - release: [record_data]
```

## Using old values to set new {#old_to_new}
### Increment value {#increment}
The set_value plugin allows to use the old values to set new values, you can use even the same
descriptor, for example to increment a scalar. 

```yaml
data:
  value_int:
    type: int
  int_array:
    size: 3
    subtype: int
    type: array
plugins:
  set_value:
    on_event:
      init:
        - share:
          - value_int: 0
          - int_array: [1, 2, 3]
      increment:
        - set:
          - value_int: "$value_int + 1"
          - int_array: ["$int_array[0] + 1", "$int_array[1] + 1", "$int_array[2] + 1"]
    on_finalize:
      - release: [value_int, int_array]
```
After calling `init` and `increment` event, `value_int` will be equal 1, and `int_array` to [2, 3, 4].

### Getting old value {#increment_in_array}
The new value is set at the end of processing the whole descriptors. This means, that if
you want to update the array element depending on other element, the old value will be set:

```yaml
metadata:
  int_array:
    size: 3
    subtype: int
    type: array
plugins:
  set_value:
    on_event:
      init:
        - expose:
          - int_array: [0, 0, 0]
      increment:
        - expose:
          - int_array: ["$int_array[0] + 1", "$int_array[0] + 1", "$int_array[1] + 1"]
```
After calling `init` and `increment` event, all values in `int_array` will equal `1`. 
This is because the `int_array[0]` was updated after setting all the elements.

### Getting new value {#update_array_after_scalar}

```yaml
metadata:
  int_scalar: int
  int_array:
    size: 3
    subtype: int
    type: array
plugins:
  set_value:
    on_event:
      init:
        - expose:
          - int_scalar: 0
          - int_array: [0, 0, 0]
      increment:
        - expose:
          - int_scalar: $int_scalar+1
          - int_array: ["$int_scalar", "$int_scalar", "$int_scalar"]
```
After calling `init` and `increment` event, all values in `int_array` will equal `1`. 
This is because the `int_scalar` is set and then the `int_array` is updated after the `int_scalar` has a new value.
