# User-code examples {#user_code_examples}

Each example demonstrates one feature of the \ref user_code_plugin "user-code plugin", which calls your own functions, with the `user_code`
part of a specification tree; the \ref user_code_reference "reference" documents each of their keys.
The whole specification trees are in `plugins/user_code/docs/` in %PDI's sources, where %PDI's tests run them with the programs and the
functions these examples describe, and check what they print.

The functions take no argument and return nothing: they access the data they work on through %PDI, under the names the specification tree
gives them.
The program must export their symbols, as \ref important_notes_node says.

## Hello world! {#hello_world_node}

\ref user_code_on_event "on_event" calls a function when the program emits an event: here, `hello_world`, which prints `Hello world!`, on the
`print` event.
The function uses no data, so its \ref user_code_function_list_function_name "aliases" are empty.

\snippet{trimleft} user_code/docs/hello_world.yml user_code

When the program emits `print`, it prints `Hello world!`.

## Handling input {#handling_input_node}

An \ref user_code_function_list_function_name_alias_name "alias" gives a function a data under the name it accesses: here, `print_number`
reads an `int` it accesses as `input`, with `PDI_IN`, and prints it.
The program shares `number`, an `int`, then emits the `print` event.

\snippet{trimleft} user_code/docs/print_number_on_event.yml user_code

With `number` at 42, `print_number` prints that it got number 42.

\ref user_code_on_data "on_data" calls the function each time a data is shared instead, so that the program only needs to expose `number`,
without an event:

\snippet{trimleft} user_code/docs/print_number_on_data.yml user_code

## Handling output {#handling_output_node}

A function writes to a data through its alias as well, with the access rights it asks for: here, `add_ten` accesses `input` with
`PDI_INOUT`, and adds 10 to it, each time the program exposes `number` with `PDI_INOUT`.

\snippet{trimleft} user_code/docs/adding_to_number.yml user_code

The program exposes `number` at 42, and gets it back at 52.

## Multiple input/output data {#multiple_inout_data_node}

A function can be given several data: here, `sum_and_multiply` reads `number1` and `number2`, and writes their sum to `sum` and their product
to `product`.
The program shares `foo`, 4, and `bar`, 5, for reading, and `res1` and `res2` for writing, with the `calculate` event.

\snippet{trimleft} user_code/docs/calculate.yml user_code

The program gets `res1` at 9 and `res2` at 20.

## Calling a function on some iterations only {#every_other_node}

A function list can have a \ref user_code_function_list_when "when" condition: here, `report`, which prints the `step` it is given, is
only called at the even iterations.
The program exposes `iter`, an `int` metadata, at each of 5 iterations.

\snippet{trimleft} user_code/docs/every_other.yml user_code

The program prints `Reporting at iteration <n>.` for 0, 2 and 4 only.
