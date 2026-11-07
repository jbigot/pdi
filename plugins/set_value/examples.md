# Set_value examples {#set_value_examples}

Each example demonstrates one feature of the \ref set_value_plugin "Set_value plugin" with the `set_value` part of a specification tree; the
\ref set_value_reference "reference" documents each of their keys.
The whole specification trees are in `plugins/set_value/docs/` in %PDI's sources, where %PDI's tests run them with the programs these
examples describe, and check what they get.

## Giving parameters to the program {#set_value_ex_params}

The plugin shares data with the values that the specification tree gives, so that the program accesses them as any other data.
Here, \ref set_value_on_init "on_init" \ref set_value_operations_item_share "shares" them in \ref PDI_init, and
\ref set_value_on_finalize "on_finalize" \ref set_value_operations_item_release "releases" them in \ref PDI_finalize.
`nb_iter` is declared as an `int` and `alpha` as a `double`, in the `data` section of the specification tree.

\snippet{trimleft} set_value/docs/params.yml set_value

Between \ref PDI_init and \ref PDI_finalize, the program can access `nb_iter`, 10, and `alpha`, 0.125, with \ref PDI_access.

A record groups the parameters in a single data, given member by member.
Here, `params` is declared as a `struct` with an `int` member `nb_iter`, a `double` member `alpha` and a member `size`, an array of 2 `int`.

\snippet{trimleft} set_value/docs/record.yml set_value

## Counting events {#set_value_ex_counter}

A data that the plugin shares can be \ref set_value_operations_item_set "set" again later, here on each `step` event
(\ref set_value_on_event_event_name "on_event"), from a $-expression that reads its current value.
`counter` is declared as an `int`, in the `data` section of the specification tree.

\snippet{trimleft} set_value/docs/counter.yml set_value

A program that emits `step` 3 times reads `counter` as 1, 2, then 3, after each of them.

## Computing new values from old ones {#set_value_ex_old_values}

The examples of this section configure a program that emits the `init` event, then the `increment` event, then reads `int_scalar`, an
`int`, and `int_array`, an array of 3 `int`.

### Incrementing a value {#set_value_ex_increment}

The new values can be computed from the old ones, even those of the data being set, for example to increment a scalar, or each element of an
array.

\snippet{trimleft} set_value/docs/increment.yml set_value

The program reads `int_scalar` as 1 and `int_array` as `[2, 3, 4]`.

### Getting the old value {#set_value_ex_old_value}

The new value of a data is only set once all its elements are evaluated.
So when an element of an array depends on another element of the same array, it gets the old value of that element.
Here, the data are metadata, which the plugin \ref set_value_operations_item_expose "exposes": %PDI keeps a copy of their last value, which the
program accesses after the plugin exposed them.

\snippet{trimleft} set_value/docs/old_value.yml set_value

The program reads `int_scalar` as 0 and `int_array` as `[1, 1, 1]`.

### Getting the new value {#set_value_ex_new_value}

The data of a \ref set_value_value_list "value list" are set in order: a data gets the new value of those set before it.

\snippet{trimleft} set_value/docs/new_value.yml set_value

The program reads `int_scalar` as 1 and `int_array` as `[1, 1, 1]`.

## Emitting events for the other plugins {#set_value_ex_events}

The examples from here on configure a program that shares `iter`, an `int` metadata, then `field`, an array of 3 `double`, at each of 2
iterations, and emits no event.
The plugin emits one, `field_ready`, with an \ref set_value_operations_item_event "event" operation each time `field` is shared
(\ref set_value_on_data_data_name "on_data"), on which the \ref Decl_HDF5_plugin "Decl'HDF5 plugin" writes it, without changing the program.

\snippet{trimleft} set_value/docs/events.yml plugins

`output.h5` holds the datasets `field_0` and `field_1`.

## Showing the iteration in the log {#set_value_ex_logger}

The plugin can change the \ref set_value_operations_item_logger_pattern "pattern" of the loggers of %PDI and of the plugins: here, to show the
current iteration, each time `iter` is shared.
With \ref set_value_operations_item_logger_evaluate "evaluate", `%{iter}` is replaced by its value each time the operation runs.
The \ref trace_plugin "Trace plugin", loaded with `trace: ~` beside `set_value` in the `plugins` section, logs what happens in %PDI, and the
global `logging` sets the pattern `[%n] %v` until the first iteration.

\snippet{trimleft} set_value/docs/logger.yml set_value

From the first share of `iter` on, each line of the log starts with the iteration, for example
`[iteration 1][Trace] =>>   data becoming available in the store: field`.

## Changing the log level {#set_value_ex_level}

The plugin can change the \ref set_value_operations_item_logger_level "level" of the logger of %PDI: here, %PDI logs what it does in detail
from the share of `iter` to that of `field` only.

\snippet{trimleft} set_value/docs/level.yml set_value

At each iteration, %PDI logs its `trace` messages between the share of `iter` and that of `field` only, such as the rights with which it
shares `field`, and the plugin warns each time it changes the level, with `Changing level to trace`, then `Changing level to info`.
