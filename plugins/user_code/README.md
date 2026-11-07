# The user-code plugin {#user_code_plugin}

The `user-code` plugin enables one to call a user-defined function 
when a specified event occur or certain data becomes available.

## Important notes {#important_notes_node}

* Make sure to compile your program with `Wl,--export-dynamic` or `-rdynamic`
  flag (in CMake set `ENABLE_EXPORTS` to `TRUE` using `set_target_properties`
  command) in order to generate necessary symbols.
* Make sure you use the proper access rights in your function in \ref PDI_access
  (PDI_IN for reading, PDI_OUT for writing).
* Descriptor aliases enables one to use different descriptors without the need
  to recompile the code.

## Dependencies between the code and the specification tree {#dependencies_node}

To ensure proper work of the `user-code` plugin, there are several conventions to follow in the application 
code and the specification tree.

First, each function name in the specification tree must be the name of a symbol of the program (watch out for [name mangling](https://en.wikipedia.org/wiki/Name_mangling)). A function cannot be named `when`: this key is reserved for the condition of a function list.

Second, these functions **can not** take any arguments or return any value (i.e. their type must be `void(void)`). Use descriptors for passing input/output variables instead.

Third, function's symbols **must** be exported to make them accessible by `user-code` plugin. To do this, compile your program as described in \ref important_notes_node.

## Use examples {#use_examples_node}

This section shows a simple examples with the use of a `user-code` plugin.

### Hello world! {#hello_world_node}

First, we will call simple function without the use of descriptors to print "Hello world!"
on "print" event.

hello_world.c:
```C
#include <stdio.h>
#include <pdi.h>

void hello_world(void)
{
    printf("Hello world!\n");
}

int main(int argc, char* argv[])
{
    PC_tree_t conf = PC_parse_path("hello_world.yml");
    PDI_init(conf);
    PDI_event("print");
    PDI_finalize();
    return 0;
}
```

hello_world.yml:
```yaml
plugins:
    user_code:
        on_event:
            print:
                hello_world: {}
```

output:
```console
[13:40:55][User-code] *** info: Plugin loaded successfully
[13:40:55][PDI] *** info: Initialization successful
Hello world!
[13:40:55][PDI] *** info: Finalization
[13:40:55][User-code] *** info: Closing plugin
```

### Handling input {#handling_input_node}

Now we will pass some input data to the function.

print_number.c:
```C
#include <stdio.h>
#include <pdi.h>

void print_number(void)
{
    void* input; PDI_access("input", &input, PDI_IN);
    printf("I've got number %d.\n", *((int*)input));
    PDI_release("input");
}

int main(int argc, char* argv[])
{
    PC_tree_t conf = PC_parse_path("print_number.yml");
    PDI_init(conf);
    int number = 42;
    PDI_share("number", &number, PDI_OUT);
    PDI_event("print");
    PDI_reclaim("number");
    PDI_finalize();
    return 0;
}
```

print_number.yml:
```yaml
data:
    number: int
plugins:
    user_code:
        on_event:
            print:
                print_number: {input: $number}
```

output:
```console
[13:49:30][User-code] *** info: Plugin loaded successfully
[13:49:30][PDI] *** info: Initialization successful
I've got number 42.
[13:49:30][PDI] *** info: Finalization
[13:49:30][User-code] *** info: Closing plugin

```

We can simplify this example by using `on_data` to print value of number
when it is shared to %PDI.

print_number.c:
```C
#include <stdio.h>
#include <pdi.h>

void print_number(void)
{
    void* input; PDI_access("input", &input, PDI_IN);
    printf("I've got number %d.\n", *((int*)input));
    PDI_release("input");
}

int main(int argc, char* argv[])
{
    PC_tree_t conf = PC_parse_path("print_number.yml");
    PDI_init(conf);
    int number = 42;
    PDI_expose("number", &number, PDI_OUT);
    //PDI_event is no longer necessary and PDI_share/PDI_reclaim can be simplified to PDI_expose
    PDI_finalize();
    return 0;
}
```

print_number.yml:
```yaml
data:
    number: int
plugins:
    user_code:
        on_data:
            number:
                print_number: {input: $number}
```

output does not change:
```console
[13:52:15][User-code] *** info: Plugin loaded successfully
[13:52:15][PDI] *** info: Initialization successful
I've got number 42.
[13:52:15][PDI] *** info: Finalization
[13:52:15][User-code] *** info: Closing plugin
```

### Handling output {#handling_output_node}

Output handling is very similar to the input handling, the only
difference are the access rights. In this example we will call
`add_ten` function when `number` is shared to %PDI.

adding_to_number.c:
```C
#include <stdio.h>
#include <pdi.h>

void add_ten(void)
{
    void* input; PDI_access("input", &input, PDI_OUT);
    *((int*)input) += 10;
    PDI_release("input");
}

int main(int argc, char* argv[])
{
    PC_tree_t conf = PC_parse_path("adding_to_number.yml");
    PDI_init(conf);
    int number = 42;
    printf("Before expose, number = %d.\n", number);
    PDI_expose("number", &number, PDI_IN);
    printf("After expose, number = %d.\n", number);
    PDI_finalize();
    return 0;
}
```

adding_to_number.yml:
```yaml
data:
    number: int
plugins:
    user_code:
        on_data:
            number:
                add_ten: {input: $number}
```

output:
```console
[13:53:51][User-code] *** info: Plugin loaded successfully
[13:53:51][PDI] *** info: Initialization successful
Before expose, number = 42.
After expose, number = 52.
[13:53:51][PDI] *** info: Finalization
[13:53:51][User-code] *** info: Closing plugin

```

### Multiple input/output data {#multiple_inout_data_node}

In this example we will use multiple data in function. We will add 
and multiply two given numbers and return the results on event "calculate".

calculate.c:
```C
#include <stdio.h>
#include <pdi.h>

void sum_and_multiply(void)
{
    void* number1; PDI_access("number1", &number1, PDI_IN);
    void* number2; PDI_access("number2", &number2, PDI_IN);
    void* sum;     PDI_access("sum", &sum, PDI_OUT);
    void* product; PDI_access("product", &product, PDI_OUT);
    *((int*)sum) = *((int*)number1) + *((int*)number2);
    *((int*)product) = *((int*)number1) * *((int*)number2);
    PDI_release("number1");
    PDI_release("number2");
    PDI_release("sum");
    PDI_release("product");
}

int main(int argc, char* argv[])
{
    PC_tree_t conf = PC_parse_path("calculate.yml");
    PDI_init(conf);
    int foo = 4, bar = 5, res1 = 0, res2 = 0;
    printf("Before calculation, foo = %d, bar = %d, res1 = %d, res2 = %d.\n", foo, bar, res1, res2);
    PDI_multi_expose("calculate", 
        "foo", &foo, PDI_OUT,
        "bar", &bar, PDI_OUT,
        "res1", &res1, PDI_IN,
        "res2", &res2, PDI_IN, NULL);
    printf("After calculation, foo = %d, bar = %d, res1 = %d, res2 = %d.\n", foo, bar, res1, res2);
    PDI_finalize();
    return 0;
}
```

calculate.yml:
```yaml
data:
    foo: int
    bar: int
    res1: int
    res2: int
plugins:
    user_code:
        on_event:
            calculate:
                sum_and_multiply: {number1: $foo, number2: $bar, sum: $res1, product: $res2}
```

output:
```console
[13:58:20][User-code] *** info: Plugin loaded successfully
[13:58:20][PDI] *** info: Initialization successful
Before calculation, foo = 4, bar = 5, res1 = 0, res2 = 0.
After calculation, foo = 4, bar = 5, res1 = 9, res2 = 20.
[13:58:20][PDI] *** info: Finalization
[13:58:20][User-code] *** info: Closing plugin
```

## `@user_code` {#user_code_config}

A mapping.
Use it, as the value of `plugins.user_code` (see \ref spectree_pdi_plugins_plugin_name), to give the functions to call, and when.
Keys not listed here are ignored.

**Example:**

```yaml
plugins:
  user_code:
    on_event:
      checkpoint:         # <- functions called on an event...
        write_checkpoint: {}
    on_data:
      field:              # <- ... and when data is shared
        process_field: {input: $field}
```


### `@user_code.on_event` {#user_code_on_event}

*Optional.*
A mapping.
Use `on_event` to call functions when events are emitted.
It can also be a sequence of single-key mappings, which has the same meaning and lets an event appear more than once.

**Example:**

```yaml
on_event:                 # <- functions called on events
  checkpoint:
    write_checkpoint: {}
```


### `@user_code.on_event.<event_name>` {#user_code_on_event_event_name}

Either a single \ref user_code_function_list or a sequence of them.
Use `on_event.<event_name>` to call the functions each time the event named `<event_name>` is emitted.

**Example:**

```yaml
on_event:
  - my_event:             # <- two function lists for the same event, in the sequence form
      when: $cond > 1
      fun1: {in: $desc2, out: $desc3}
  - my_event:
      fun2: {}
```

`fun2` is called each time `my_event` is emitted, `fun1` only if, in addition, `$cond > 1`.


### `@user_code.on_data` {#user_code_on_data}

*Optional.*
A mapping.
Use `on_data` to call functions when data are shared.
It can also be a sequence of single-key mappings, which has the same meaning and lets a data appear more than once.

**Example:**

```yaml
on_data:                  # <- functions called when data is shared
  field:
    process_field: {input: $field}
```


### `@user_code.on_data.<data_name>` {#user_code_on_data_data_name}

Either a single \ref user_code_function_list or a sequence of them.
Use `on_data.<data_name>` to call the functions each time the data named `<data_name>` is shared.

**Example:**

```yaml
on_data:
  field:                  # <- called each time field is shared
    process_field: {input: $field}
    report: {}
```


### `@user_code.logging` {#user_code_logging}

*Optional.*
A \ref spectree_logging.
Use `logging` to configure the logger of the plugin, see \ref spectree_pdi_plugins_plugin_name_logging.

**Example:**

```yaml
logging: debug            # <- the logger of the User-code plugin
on_event:
  checkpoint:
    write_checkpoint: {}
```


## `@function_list` {#user_code_function_list}

A *function list*: a mapping with the keys below, giving the functions to call, in order, and an optional condition.
You can use it in \ref user_code_on_event_event_name and \ref user_code_on_data_data_name.

**Example:**

```yaml
on_event:
  step:                   # <- this mapping is a function list
    when: $iter > 0
    compute: {in: $field, out: $result}
    report: {}
```


### `@function_list.when` {#user_code_function_list_when}

*Optional.*
A \ref spectree_expression.
Use `when` to call the functions of the list only under a condition.
It is evaluated for each function, once its aliases are exposed, and the function is only called if it is true.
An error while evaluating it is logged and does not interrupt the program.

**Example:**

```yaml
when: $iter % 10 = 0      # <- the functions are only called every 10 iterations
report: {}
```


### `@function_list.<function_name>` {#user_code_function_list_function_name}

A mapping, empty (`{}`) when the function needs no alias.
Use `<function_name>` to call the function named `<function_name>`, with the aliases to expose while it runs.
The function must be of type `void(void)` and its symbol exported, see \ref dependencies_node.
A function cannot be named `when`: this key is reserved for the condition of the list.

**Example:**

```yaml
compute: {in: $field}     # <- calls compute(), with field shared as in while it runs
report: {}                # <- calls report(), with nothing shared
```


### `@function_list.<function_name>.<alias_name>` {#user_code_function_list_function_name_alias_name}

A \ref spectree_expression.
Use `<alias_name>` to share its value under the name `<alias_name>` while the function runs.

**Example:**

```yaml
compute:
  in: $field              # <- compute() accesses field as in
  out: $result            #    and result as out
```


## Specification tree example {#full_spec_tree_example_node}

```yaml
metadata:
    cond: int
data:
    desc1: int                          
    desc2: float                   
    desc3: double                  
plugins:                       
    user_code:                   
        on_data:                   
            desc1:                 
                when: '$cond>1'
                fun1: {in: $desc2, out: $desc3}
            desc2:
                fun2: {}
                fun3: {out: $desc2}
        on_event:                  
            event1:                 
                when: '$cond<1'     
                fun2: {}
            event2:
                fun4: {param1: $desc2, param2: $desc1, param3: $desc3}
```
