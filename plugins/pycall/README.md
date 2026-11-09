# Pycall plugin {#pycall_plugin}

The Pycall plugin allows calling python scripts from yaml file, that can work either if wanted data was shared or program calls specific event.

## Configuration {#pycall_configuration}

Simple plugin build:
```yaml
plugins:
  pycall:           # name of the plugin
    on_event:       # run code when an event is emitted
      testing:      # name of the event
        with:       # values exposed to the code, and their Python names
        exec:       # Python code to execute when the event is emitted
    on_data:        # run code when a data is shared
      data_name:    # Python code to execute when data_name is shared
```

## Examples {#pycall_ex}

A full example of the Pycall plugin, with its specification tree and the C code that uses it.

Specification tree:
```yaml
logging: trace
data: {a: {type: array, subtype: int, size: 3}}
plugins:
  pycall:
    on_event:
      testing:
        with: { a_python: $a }
        exec: "print(' * [P] I received    $a =',a_python); a_python[1]=7; print(' * [P] changed it to $a =',a_python);"
```

C code:
```c
	int a[3] = {1, 2, 3};
	
	printf(" * [C] starting with $a = [%d %d %d]\n", a[0], a[1], a[2]);
	
	PDI_share("a", a, PDI_INOUT);
	PDI_event("testing");
	PDI_reclaim("a");
	
	printf(" * [C] now I see     $a = [%d %d %d]\n", a[0], a[1], a[2]);
```
Output of the program:
```
 * [C] starting with $a = [1 2 3]
[13:51:26][PDI] *** trace: Sharing `a' Ref with rights: R = true, W = true
[13:51:26][PDI] *** trace: Calling `a' share. Callbacks to call: 0
[13:51:26][PDI] *** trace: Calling `testing' event. Callbacks to call: 1
 * [P] I received    $a = [1 2 3]
 * [P] changed it to $a = [1 7 3]
 * [C] now I see     $a = [1 7 3]
```

## `@pycall` {#pycall_config}

A mapping.
Use it, as the value of `plugins.pycall` (see \ref spectree_pdi_plugins_plugin_name), to give the Python code to execute, and when.

The code is executed in the scope of the `__main__` Python module, where the `pdi` Python module is also available under the name `pdi`.
Variables defined by the code, and values exposed to it, remain in that scope and are visible to the code executed afterwards.
An exception raised by the Python code is logged as an error; it does not interrupt the program.

**Example:**

```yaml
plugins:
  pycall:
    on_event:
      my_event:           # <- code run when my_event is emitted...
        exec: "print('my_event was emitted')"
    on_data:
      my_data: "print(my_data)"   # <- ... and when my_data is shared
```


### `@pycall.on_event.<event_name>` {#pycall_on_event_event_name}

Either a single \ref pycall_pycode or a sequence of them.
Use `on_event.<event_name>` to execute the code, in order, each time the event named `<event_name>` is emitted.

**Example:**

```yaml
on_event:
  testing:                # <- a single piece of code...
    exec: "print(a_python); a_python[1] = 7"
    with: {a_python: $a}
```

The code sees the value of `a` as `a_python`, and can change it if `a` is shared with write access.


**Example:**

```yaml
on_event:
  testing:                # <- ... or a sequence of them, run in order
    - exec: "print(x)"
      with: {x: $a}
    - exec: "print(y)"
      with: {y: $b}
```


### `@pycall.on_data.<data_name>` {#pycall_on_data_data_name}

A scalar.
Use `on_data.<data_name>` to execute this Python code each time the data named `<data_name>` is shared.
The data is exposed to the code as the variable named `<data_name>`.

**Example:**

```yaml
on_data:
  a: "print(' * [P] I received $a =', a)"   # <- run each time a is shared, which it sees as a
```


### `@pycall.logging` {#pycall_logging}

*Optional.*
A \ref spectree_logging.
Use `logging` to configure the logger of the plugin, see \ref spectree_pdi_plugins_plugin_name_logging.

**Example:**

```yaml
logging: debug            # <- the logger of the Pycall plugin
on_data:
  a: "print(a)"
```


## `@pycode` {#pycall_pycode}

A piece of Python code to execute, and the values it sees: a mapping with the keys below.
You can use it in \ref pycall_on_event_event_name.

**Example:**

```yaml
on_event:
  my_event:
    exec: "print(x * 2)"  # <- this mapping is a piece of code, and the values it sees
    with: {x: $my_data}
```


### `@pycode.exec` {#pycall_pycode_exec}

*Required.*
A scalar.
Use `exec` to give the Python code to execute.

**Example:**

```yaml
exec: "import numpy; print(numpy.sum(a))"   # <- the Python code
with: {a: $my_array}
```


### `@pycode.with.<python_name>` {#pycall_pycode_with_python_name}

*Optional.*
A \ref spectree_expression.
Use `with.<python_name>` to expose its value to the Python code as the variable named `<python_name>`.
`with` can be omitted: no value is then exposed for this execution of the code.

**Example:**

```yaml
exec: "print(x, n)"
with:
  x: $my_data             # <- the value of my_data, seen as x
  n: $size
```


