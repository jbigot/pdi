# JSON plugin {#JSON_plugin}

**WARNING** This documentation is a work in progress and does not
reflect the full potential of the JSON plugin.

The JSON plugin exports data in valid JSON, which can then be turned into custom formats (\ref json_ex_custom_format).

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

## Documentation {#json_documentation}

* \subpage JSON_examples "JSON examples" demonstrates its features one at a time, each with the part of the specification tree that configures it.
* \subpage JSON_reference "JSON configuration reference" documents each key of its configuration.
