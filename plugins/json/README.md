# JSON plugin {#JSON_plugin}

**WARNING** This documentation is a work in progress and does not
reflect the full potential of the JSON plugin.

The JSON plugin exports data in valid JSON, which can then be turned into custom formats (\ref json_custom_format).

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

## The JSON plugin for custom formatting {#json_custom_format}

For the sake of the example, let's imagine the following code, which runs a heat transfer simulation.

### The PDI calls

<!-- \image html json_simulation_code_light.png width=800px -->

```C
    PDI_multi_expose("init",
        "simulation_name", &simulation_name, PDI_OUT,
        "max_steps", &max_steps, PDI_OUT,
        "mesh_config", &mesh_config, PDI_OUT,
        "rank", &rank, PDI_OUT,
        NULL) ;

    // main loop
    for (int step = 0; step < nb_steps; ++step) {
        do_compute(temp, MPI_COMM_WORLD) ;

        // share data at every iteration
        PDI_multi_expose("iter",
            "step", &step, PDI_OUT,
            "temp", temp, PDI_OUT,
            NULL) ;
        MPI_Barrier(MPI_COMM_WORLD);
        ...
    }
```

### The related YAML configuration
<!-- \image html simulation_yaml_light.png width=800px -->

```yaml
metadata: {rank: int, step: int}
data:
    simulation_name: { type: array, subtype: char, size: 512 }
    max_steps: int
    mesh_config:
        type: struct
        members:
        - dimensions: { type: array, subtype: int, size: 3}
        - spacings: { type: array, subtype: double, size: 3}

    temp: # the main temperature field
        type: array
        subtype: double
        size: ['$mesh_config.dimensions[0]', '$mesh_config.dimensions[1]', '$mesh_config.dimensions[2]']

plugins:
    json:
    - file: data-$rank.json
      write: [simulation_name, max_steps, mesh_config, rank, step, temp]
```

### JSON output
Running this code as is generates the following JSON file, with one object per variable written.

<!-- \image html json_json_light.png width=800px -->

```json
[
{
    "simulation_name": "Heat_transfer_simulation"
},
{
    "max_steps": 10
},
{
    "mesh_config": {
        "dimensions": [100, 100, 50],
        "spacings": [0.1, 0.1, 0.1]
    }
},
{
    "rank": 7
},
{
    "step": 0
},
{
    "temp": [75.3, 74.7, 76.1, ..., 54.2, 55.6]
},
...
]
```

We can use this JSON output file to generate our custom format, for example with [mustache](https://mustache.github.io/), once the objects have
been gathered into a single one, with the iterations under a `temperature_data` key.

### Mustache code
<!-- \image html json_mustache_light.png width=800px -->

```mstch
Simulation {{simulation_name}} of {{max_steps}} iterations.
Mesh configuration was of {{mesh_config.dimensions}} dimensions and {{mesh_config.spacings}} spacings
{{#temperature_data}}
At step {{step}}, temp was [{{#temp}}{{.}}{{#last}}{{^last}},{{/last}}{{/temp}}]
{{/temperature_data}}
```

Finally, let's print it! We run the JSON to mustache converter...

```bash
vendor/mustach/mustach data.json format.mstch
```

... and get the following result!

<!-- \image html json_mustache_output_light.png width=800px -->

```text
Simulation Heat_transfer_simulation of 10 iterations.
Mesh configuration was of [100, 100, 50] dimensions and [0.1, 0.1, 0.1] spacings
At step 1, temp was [75.3, 74.7, 76.1, ..., 54.2, 55.6]
At step 2, temp was [91.8, 89.6, 92.7, ..., 12.5, 13.7]
...
```