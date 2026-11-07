# JSON configuration reference {#JSON_reference}

This page documents the configuration of the \ref JSON_plugin "JSON plugin", one section per key, named after its path as in the
\ref Specification_tree_ref "specification tree reference".
\ref JSON_examples "JSON examples" shows how to combine them.

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
