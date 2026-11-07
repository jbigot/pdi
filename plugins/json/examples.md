# JSON examples {#JSON_examples}

Each example demonstrates one feature of the \ref JSON_plugin "JSON plugin" with the `json` part of a specification tree; the
\ref JSON_reference "reference" documents each of their keys.
The whole specification trees are in `plugins/json/docs/` in %PDI's sources, where %PDI's tests run them and check the files they write.

## The data {#json_ex_program}

All the examples configure the same program, which shares these data with %PDI:

| data   | type                                                   | shared                                                  |
|:-------|:-------------------------------------------------------|:--------------------------------------------------------|
| `mesh` | metadata record of `dimensions`, an array of 2 `int`, and `spacings`, an array of 2 `double` | once, at the start: dimensions `[2, 3]`, spacings `[0.5, 0.25]` |
| `step` | `int` metadata                                         | at each step, with `temp` and the `iteration` event: 0, 1, 2 |
| `temp` | array of 2x3 `double`                                  | at each step, with `step` and the `iteration` event: `10 * step + 3 * row + column` |

## Writing a data to its own file {#json_ex_one}

A file description of the \ref json_jsonfile_data_name "<data_name>" form writes one data to one file, each time it is shared.

\snippet{trimleft} json/docs/one.yml json

`step.json` holds an array, with one object per write: `[{"step": 0}, {"step": 1}, {"step": 2}]`.

## Writing several data to one file, conditionally {#json_ex_several}

A file description of the \ref json_jsonfile_file "file" form \ref json_jsonfile_write "writes" several data to one file, here only at the
even steps, as \ref json_jsonfile_when "when" says.
A sequence of file descriptions writes several files.

\snippet{trimleft} json/docs/several.yml json

Records and arrays, even nested, become JSON objects and arrays.
`setup.json` holds `[{"mesh": {"dimensions": [2, 3], "spacings": [0.5, 0.25]}}]`.
`steps.json` holds 4 objects, one per write, in order: `{"step": 0}`, then `{"temp": [[0.0, 1.0, 2.0], [3.0, 4.0, 5.0]]}`, then
`{"step": 2}`, then the `temp` of step 2.

## Turning the output into a format of your own {#json_ex_custom_format}

Any JSON tool can turn these files into the format you need: a [Mustache](https://mustache.github.io/) template, a Python script, or
[jq](https://jqlang.org/).
Since each write is an object of its own, a step and the field that follows it in `steps.json` are paired by their position in the array.
