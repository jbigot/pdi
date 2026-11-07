# The user-code plugin {#user_code_plugin}

The `user-code` plugin calls user-defined functions when a given event is
emitted, or when some data becomes available.

## Important notes {#important_notes_node}

* Make sure to compile your program with the `-Wl,--export-dynamic` or `-rdynamic`
  flag (in CMake, set the `ENABLE_EXPORTS` property to `TRUE` with the
  `set_target_properties` command), so that its symbols are exported.
* Make sure to use the proper access rights in the calls to \ref PDI_access of your
  functions (`PDI_IN` for reading, `PDI_OUT` for writing, `PDI_INOUT` for both).
* Aliases make it possible to pass different data to a function without
  recompiling the code.

## Dependencies between the code and the specification tree {#dependencies_node}

For the `user-code` plugin to work properly, the application code and the specification tree must follow
several conventions.

First, each function name in the specification tree must be the name of a symbol of the program (watch out for [name mangling](https://en.wikipedia.org/wiki/Name_mangling)). A function cannot be named `when`: this key is reserved for the condition of a function list.

Second, these functions **cannot** take any argument or return any value (*i.e.* their type must be `void(void)`). Use aliases to pass input and output variables instead.

Third, the symbols of the functions **must** be exported, for the `user-code` plugin to find them. To do so, compile your program as described in \ref important_notes_node.

## Documentation {#user_code_documentation}

* \subpage user_code_examples "User-code examples" demonstrates its features one at a time, with whole programs and specification trees.
* \subpage user_code_reference "User-code configuration reference" documents each key of its configuration.
