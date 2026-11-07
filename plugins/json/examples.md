# JSON examples {#JSON_examples}

Each example demonstrates one feature of the \ref JSON_plugin "JSON plugin", with a whole specification tree that you can run as is; the
\ref JSON_reference "reference" documents each of their keys.
The last one turns its output into a format of your own.

## The program {#json_ex_program}

All the examples use the same program: only their specification tree differs.
It shares the description of its mesh, then a temperature field at each of 3 steps.

```c
#include <pdi.h>

struct mesh {
	int dimensions[2];
	double spacings[2];
};

int main(int argc, char* argv[])
{
	PDI_init(PC_parse_path(argv[1]));

	struct mesh mesh = {.dimensions = {2, 3}, .spacings = {0.5, 0.25}};
	PDI_expose("mesh", &mesh, PDI_OUT);

	double temp[2][3];
	for (int step = 0; step < 3; ++step) {
		for (int i = 0; i < 2; ++i) {
			for (int j = 0; j < 3; ++j) temp[i][j] = 10 * step + 3 * i + j;
		}
		PDI_multi_expose("iteration",
		    "step", &step, PDI_OUT,
		    "temp", temp, PDI_OUT,
		    NULL);
	}

	PDI_finalize();
	return 0;
}
```

Compile it as described in \ref compilation_of__app, and run it with the specification tree as its argument, in an environment set up for
%PDI (see \ref running_app):

```bash
./main one.yml
```

## Writing a data to its own file {#json_ex_one}

A file description of the `<data_name>` form writes one data to one file, each time it is shared.

```yaml
metadata:
  step: int
data:
  temp: {type: array, subtype: double, size: [2, 3]}
plugins:
  json:
    step: step.json            # step, to step.json
```

The file holds an array, with one object per write.

step.json:
```json
[
{
    "step": 0
},
{
    "step": 1
},
{
    "step": 2
}
]
```

## Writing several data to one file, conditionally {#json_ex_several}

A file description of the `file` form writes several data to one file, here only at the even steps.
A sequence of file descriptions writes several files.
Records and arrays, even nested, become JSON objects and arrays.

```yaml
metadata:
  step: int
  mesh:
    type: struct
    members:
      - dimensions: {type: array, subtype: int, size: 2}
      - spacings: {type: array, subtype: double, size: 2}
data:
  temp:
    type: array
    subtype: double
    size: ['$mesh.dimensions[0]', '$mesh.dimensions[1]']
plugins:
  json:
    - file: setup.json
      write: [mesh]
    - file: steps.json
      when: $step % 2 = 0        # only at the even steps
      write: [step, temp]
```

setup.json:
```json
[
{
    "mesh": {
        "dimensions": [
            2,
            3
        ],
        "spacings": [
            0.5,
            0.25
        ]
    }
}
]
```

steps.json:
```json
[
{
    "step": 0
},
{
    "temp": [
        [
            0.0,
            1.0,
            2.0
        ],
        [
            3.0,
            4.0,
            5.0
        ]
    ]
},
{
    "step": 2
},
{
    "temp": [
        [
            20.0,
            21.0,
            22.0
        ],
        [
            23.0,
            24.0,
            25.0
        ]
    ]
}
]
```

## Turning the output into a format of your own {#json_ex_custom_format}

Any JSON tool can turn these files into the format you need: a [Mustache](https://mustache.github.io/) template, a Python script, or
[jq](https://jqlang.org/), as here.
Since each write is an object of its own, the step and the field that follows it are paired by their position in the array.

```text
$ jq -r '.[0].mesh
    | "\(.dimensions | join("x")) mesh, spacings \(.spacings | join(", "))"' \
    setup.json
2x3 mesh, spacings 0.5, 0.25
$ jq -r '[.[] | to_entries[0]] as $w
    | range(0; $w | length; 2)
    | "step \($w[.].value): temp = \($w[. + 1].value | flatten)"' \
    steps.json
step 0: temp = [0.0,1.0,2.0,3.0,4.0,5.0]
step 2: temp = [20.0,21.0,22.0,23.0,24.0,25.0]
```
