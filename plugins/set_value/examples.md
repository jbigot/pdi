# Set_value examples {#set_value_examples}

Each example demonstrates one feature of the \ref set_value_plugin "Set_value plugin", with a whole program and specification tree that you
can run as is; the \ref set_value_reference "reference" documents each of their keys.

Compile each program as described in \ref compilation_of__app, and run it with the specification tree as its argument, in an environment set up
for %PDI (see \ref running_app):

```bash
./params params.yml
```

The outputs shown leave out the messages that %PDI and the plugins log by default, but in \ref set_value_ex_logger and
\ref set_value_ex_level, which are about them.

## Giving parameters to the program {#set_value_ex_params}

The plugin shares data with the values that the specification tree gives, so that the program accesses them as any other data.
It shares them in \ref PDI_init, and releases them in \ref PDI_finalize.

```c
#include <stdio.h>
#include <pdi.h>

int main(int argc, char* argv[])
{
	PDI_init(PC_parse_path(argv[1]));

	// the parameters, that the specification tree gives
	int* nb_iter;
	double* alpha;
	PDI_access("nb_iter", (void**)&nb_iter, PDI_IN);
	PDI_access("alpha", (void**)&alpha, PDI_IN);
	printf("running %d iterations with alpha = %g\n", *nb_iter, *alpha);
	PDI_release("alpha");
	PDI_release("nb_iter");

	PDI_finalize();
	return 0;
}
```

```yaml
data:
  nb_iter: int
  alpha: double
plugins:
  set_value:
    on_init:                     # in PDI_init
      - share:
          - nb_iter: 10
          - alpha: 0.125
    on_finalize:                 # in PDI_finalize
      - release: [nb_iter, alpha]
```

```text
$ ./params params.yml
running 10 iterations with alpha = 0.125
```

A record groups them in a single data, given member by member:

```c
#include <stdio.h>
#include <pdi.h>

struct params {
	int nb_iter;
	double alpha;
	int size[2];
};

int main(int argc, char* argv[])
{
	PDI_init(PC_parse_path(argv[1]));

	// the parameters, that the specification tree gives
	struct params* params;
	PDI_access("params", (void**)&params, PDI_IN);
	printf("running %d iterations with alpha = %g on a %dx%d grid\n",
	       params->nb_iter, params->alpha, params->size[0], params->size[1]);
	PDI_release("params");

	PDI_finalize();
	return 0;
}
```

```yaml
data:
  params:
    type: struct
    members:
      - nb_iter: int
      - alpha: double
      - size: {type: array, subtype: int, size: 2}
plugins:
  set_value:
    on_init:
      - share:
          - params:              # a record, member by member
              nb_iter: 10
              alpha: 0.125
              size: [60, 12]
    on_finalize:
      - release: [params]
```

```text
$ ./record record.yml
running 10 iterations with alpha = 0.125 on a 60x12 grid
```

## Counting events {#set_value_ex_counter}

A data that the plugin shares can be set again later, here at each `step` event, from a $-expression that reads its current value.

```c
#include <stdio.h>
#include <pdi.h>

int main(int argc, char* argv[])
{
	PDI_init(PC_parse_path(argv[1]));

	for (int iter = 0; iter < 3; ++iter) {
		PDI_event("step");
		int* counter;
		PDI_access("counter", (void**)&counter, PDI_IN);
		printf("after step %d, counter = %d\n", iter, *counter);
		PDI_release("counter");
	}

	PDI_finalize();
	return 0;
}
```

```yaml
data:
  counter: int
plugins:
  set_value:
    on_init:
      - share:
          - counter: 0
    on_event:
      step:                      # at each step event
        - set:
            - counter: $counter + 1
    on_finalize:
      - release: [counter]
```

```text
$ ./counter counter.yml
after step 0, counter = 1
after step 1, counter = 2
after step 2, counter = 3
```

## Computing new values from old ones {#set_value_ex_old_values}

The examples of this section all use this program, which emits the `init` and `increment` events, then prints the values that the plugin set.

```c
#include <stdio.h>
#include <pdi.h>

int main(int argc, char* argv[])
{
	PDI_init(PC_parse_path(argv[1]));

	PDI_event("init");
	PDI_event("increment");

	int* value;
	PDI_access("int_scalar", (void**)&value, PDI_IN);
	printf("int_scalar = %d\n", *value);
	PDI_release("int_scalar");
	int* array;
	PDI_access("int_array", (void**)&array, PDI_IN);
	printf("int_array = [%d, %d, %d]\n", array[0], array[1], array[2]);
	PDI_release("int_array");

	PDI_finalize();
	return 0;
}
```

### Incrementing a value {#set_value_ex_increment}

The new values can be computed from the old ones, even those of the data being set, for example to increment a scalar, or each element of an
array.

```yaml
data:
  int_scalar: int
  int_array: {type: array, subtype: int, size: 3}
plugins:
  set_value:
    on_event:
      init:
        - share:
            - int_scalar: 0
            - int_array: [1, 2, 3]
      increment:
        - set:
            - int_scalar: $int_scalar + 1
            - int_array:
                - $int_array[0] + 1
                - $int_array[1] + 1
                - $int_array[2] + 1
    on_finalize:
      - release: [int_scalar, int_array]
```

```text
$ ./values increment.yml
int_scalar = 1
int_array = [2, 3, 4]
```

### Getting the old value {#set_value_ex_old_value}

The new value of a data is only set once all its elements are evaluated.
So when an element of an array depends on another element of the same array, it gets the old value of that element.
Here, the data are metadata: %PDI keeps a copy of their last value, which the program accesses after the plugin exposed them.

```yaml
metadata:
  int_scalar: int
  int_array: {type: array, subtype: int, size: 3}
plugins:
  set_value:
    on_event:
      init:
        - expose:
            - int_scalar: 0
            - int_array: [0, 0, 0]
      increment:
        - expose:
            - int_array:
                - $int_array[0] + 1
                - $int_array[0] + 1  # int_array[0] is still 0
                - $int_array[1] + 1  # and so is int_array[1]
```

```text
$ ./values old_value.yml
int_scalar = 0
int_array = [1, 1, 1]
```

### Getting the new value {#set_value_ex_new_value}

The data of a value list are set in order: a data gets the new value of those set before it.

```yaml
metadata:
  int_scalar: int
  int_array: {type: array, subtype: int, size: 3}
plugins:
  set_value:
    on_event:
      init:
        - expose:
            - int_scalar: 0
            - int_array: [0, 0, 0]
      increment:
        - expose:
            - int_scalar: $int_scalar + 1
            - int_array:         # int_scalar is already 1
                - $int_scalar
                - $int_scalar
                - $int_scalar
```

```text
$ ./values new_value.yml
int_scalar = 1
int_array = [1, 1, 1]
```

## Emitting events for the other plugins {#set_value_ex_events}

The program below shares `iter` and `field` one after the other, and emits no event.
The plugin emits one, `field_ready`, each time `field` is shared, on which the \ref Decl_HDF5_plugin "Decl'HDF5 plugin" writes it, without
changing the program.

```c
#include <pdi.h>

int main(int argc, char* argv[])
{
	PDI_init(PC_parse_path(argv[1]));

	double field[3];
	for (int iter = 0; iter < 2; ++iter) {
		for (int i = 0; i < 3; ++i) field[i] = 10 * iter + i;
		PDI_expose("iter", &iter, PDI_OUT);
		PDI_expose("field", field, PDI_OUT);
	}

	PDI_finalize();
	return 0;
}
```

```yaml
metadata:
  iter: int
data:
  field: {type: array, subtype: double, size: 3}
plugins:
  set_value:
    on_data:
      field:                     # each time field is shared
        - event: field_ready
  decl_hdf5:
    file: output.h5
    on_event: field_ready
    write:
      field:
        dataset: field_${iter}
```

```text
$ ./loop events.yml
$ h5ls output.h5
field_0                  Dataset {3}
field_1                  Dataset {3}
```

## Showing the iteration in the log {#set_value_ex_logger}

The plugin can change the pattern of the loggers of %PDI and of the plugins: here, to show the current iteration, each time `iter` is shared.
With `evaluate`, `%{iter}` is replaced by its value each time the operation runs.
The \ref trace_plugin "Trace plugin" logs what happens in %PDI, including the `*_timer` events that %PDI emits for the
\ref timer_plugin "Timer plugin".
This example runs the program of \ref set_value_ex_events.

```yaml
logging: {pattern: '[%n] %v'}    # before the first iteration
metadata:
  iter: int
data:
  field: {type: array, subtype: double, size: 3}
plugins:
  trace: ~
  set_value:
    on_data:
      iter:                      # each time iter is shared
        - logger:
            pattern: '[iteration %{iter}][%n] %v'
            evaluate: 1
```

```text
$ ./loop logger.yml
[Set-value] Plugin loaded successfully
[Trace] Welcome!
[PDI] Initialization successful
[Trace] !!!                            named event: pdi_start_timer
[iteration 0][Trace] =>>   data becoming available in the store: iter
[iteration 0][Trace] <<= data stop being available in the store: iter
[iteration 0][Trace] =>>   data becoming available in the store: field
[iteration 0][Trace] <<= data stop being available in the store: field
[iteration 1][Trace] =>>   data becoming available in the store: iter
[iteration 1][Trace] <<= data stop being available in the store: iter
[iteration 1][Trace] =>>   data becoming available in the store: field
[iteration 1][Trace] <<= data stop being available in the store: field
[iteration 1][Trace] !!!                            named event: pdi_stop_timer
[iteration 1][PDI] Finalization
[iteration 1][Trace] Goodbye!
[iteration 1][Set-value] Closing plugin
```

## Changing the log level {#set_value_ex_level}

The plugin can change the level of the logger of %PDI: here, %PDI logs what it does in detail from the share of `iter` to that of `field`
only, and the plugin warns each time it changes the level.
This example runs the program of \ref set_value_ex_events.

```yaml
logging: {pattern: '[%n] %l: %v'}
metadata:
  iter: int
data:
  field: {type: array, subtype: double, size: 3}
plugins:
  set_value:
    on_data:
      iter:
        - logger:
            level: trace         # from iter's share
      field:
        - logger:
            level: info          # to field's
```

```text
$ ./loop level.yml
[Set-value] info: Plugin loaded successfully
[PDI] info: Initialization successful
[Set-value] warning: Changing level to trace
[PDI] trace: Calling `iter' data remove. Callbacks to call: 0
[PDI] trace: Sharing `field' Ref with rights: R = true, W = false
[PDI] trace: Calling `field' share. Callbacks to call: 1
[Set-value] warning: Changing level to info
[Set-value] warning: Changing level to trace
[PDI] trace: Calling `iter' data remove. Callbacks to call: 0
[PDI] trace: Sharing `field' Ref with rights: R = true, W = false
[PDI] trace: Calling `field' share. Callbacks to call: 1
[Set-value] warning: Changing level to info
[PDI] info: Finalization
[Set-value] info: Closing plugin
```
