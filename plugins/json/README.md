# JSON plugin {#JSON_plugin}

**WARNING** This documentation is a work in progress and does not
reflect the full potential of the JSON plugin.

The JSON plugin exports data in valid JSON, which can then be turned into custom formats (\ref json_ex_custom_format).

This page is the reference of its configuration, one section per key.
\subpage JSON_examples "JSON examples" shows, one approach at a time, how to combine them into whole specification trees.

It can write several variables to the same file, each as soon as it is available.

It supports not only the basic datatypes (scalars, arrays, records, tuples and pointers), but also their nested combinations: multi-level arrays, multi-level records, and any mixture of the supported types at any level of nesting. Not all combinations are fully tested, however.

Thanks to its incremental writing, the file is valid JSON at all times, even if the simulation crashes before PDI is finalized. **Warning:** this increases the disk bandwidth used, and may not suit a performance-sensitive context.
<!-- You might then want to consider using the `xxxxx` option to use the open file once behavior, instead of rewriting at each call. This would cause invalid JSON if simulation crashes or PDI's finalization is not called. But then you are using JSON anyways ... -->

Conditional writing is supported.

A variable is only written when it is shared with PDI with read access.

Each variable listed in the `write` section is written to the file whenever PDI is granted read access to it.

The JSON plugin cannot yet wait for several variables to be readable before writing them together.

## Example of a JSON configuration {#json_configuration_grammar}

```yaml
plugins:
  json:
    - data1: file_path.json
    - file: file_path_${iteration}.json
      when: $iteration % 10 = 0
      write: [data2, data3]
```

More examples are given in `example/json.yml`.

## `@json` {#json_config}

Either a single \ref json_jsonfile or a sequence of them.
Use it, as the value of `plugins.json` (see \ref spectree_pdi_plugins_plugin_name), to describe the writes to execute.

The file holds a JSON array, with one object per write, whose only key is the name of the variable.
If the file already exists, the new objects are appended to its array.

A variable is written each time it is shared with read permission.

**Example:**

```yaml
plugins:
  json:                   # <- a single file description...
    my_data: my_data.json
```


**Example:**

```yaml
plugins:
  json:                   # <- ... or a sequence of them
    - my_data: my_data.json
    - file: all.json
      write: [data1, data2]
```


### `@json.logging` {#json_logging}

*Optional.*
A \ref spectree_logging.
Use `logging` to configure the logger of the plugin, see \ref spectree_pdi_plugins_plugin_name_logging.
It is only taken into account when `@json` is a single \ref json_jsonfile, not a sequence.
The plugin itself does not expect it: in a file description whose first key is `file`, it is an error, and as the first key of a file
description, it is read as a \ref json_jsonfile_data_name.
It can therefore only follow the data name of a single file description of the `<data_name>` form.

**Example:**

```yaml
my_data: my_data.json
logging: debug          # <- after the data name
```

## `@jsonfile` {#json_jsonfile}

The writes to one file, either:
* a mapping with a single key, \ref json_jsonfile_data_name, that writes one variable,
* a mapping whose first key is \ref json_jsonfile_file, with the \ref json_jsonfile_write and \ref json_jsonfile_when keys, and no
  other key.
You can use it in \ref json_config.

**Example:**

```yaml
plugins:
  json:
    - my_data: my_data.json   # <- a file description of the <data_name> form
    - file: all.json          # <- a file description of the file form
      write: [data1, data2]
```


### `@jsonfile.<data_name>` {#json_jsonfile_data_name}

A \ref spectree_expression.
Use `<data_name>` to write the variable named `<data_name>` to the file at this path.

**Example:**

```yaml
step: step_${rank}.json   # <- writes step to a file per process
```


### `@jsonfile.file` {#json_jsonfile_file}

*Required, first key of its mapping.*
A \ref spectree_expression.
Use `file` to give the path of the file where to write the variables listed in \ref json_jsonfile_write.

**Example:**

```yaml
file: output_${rank}.json # <- first key: this file description is of the file form
write: [temp, step]
```


### `@jsonfile.write` {#json_jsonfile_write}

*Required*: without it, nothing is written to the file.
A sequence of scalars.
Use `write` to name the variables to write to \ref json_jsonfile_file.

**Example:**

```yaml
file: output.json
write: [temp, step]       # <- each written when it is shared
```


### `@jsonfile.when` {#json_jsonfile_when}

*Optional.*
A \ref spectree_expression.
Use `when` to set a condition, evaluated each time a variable of \ref json_jsonfile_write is about to be written.
The variable is only written if the condition is true.
By default, the variables are written each time they are shared with read permission.

**Example:**

```yaml
file: output.json
when: $step % 10 = 0      # <- one share out of ten
write: [temp]
```


## Installing and testing the JSON plugin {#json_test_plugin}
The JSON plugin relies on the [nlohmann/json](https://github.com/nlohmann/json) library to output data to JSON format.
This library is included in the PDI distribution, and is built along with the plugin when it is not found on the system.
To build the plugin, add the `BUILD_JSON_PLUGIN=ON` option to the PDI configuration command.

Once the plugin is installed, the easiest way to test it is the example of `example/`, a full, somewhat realistic C simulation code that uses PDI with the JSON plugin.

To use it, first build PDI with CMake, with `BUILD_TESTING` set to `ON`, then run:
```bash
build/PDI_EXAMPLE/PDI_example_C example/json.yml
```

<!-- ### Example output -->

## Full configuration file example {#json_config_example}

See `example/json.yml`.

## Ongoing work and limitations {#json_on_going_work}

- Reading variables:
  the JSON plugin can only write variables, not read them yet.
- Writing several variables in a single JSON object, built on the fly.
