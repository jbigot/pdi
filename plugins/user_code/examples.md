# User-code examples {#user_code_examples}

Each example demonstrates one feature of the \ref user_code_plugin "user-code plugin", which calls your own functions, with a whole program
and specification tree that you can run as is; the \ref user_code_reference "reference" documents each of their keys.

Compile each program as described in \ref compilation_of__app, with its symbols exported, as \ref important_notes_node says, and run it in an
environment set up for %PDI (see \ref running_app).
Each program reads the specification tree from the file whose name it gives to `PC_parse_path`.

## Hello world! {#hello_world_node}

First, we call a simple function, without any alias, to print "Hello world!"
on the "print" event.

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

Output:
```console
[13:40:55][User-code] *** info: Plugin loaded successfully
[13:40:55][PDI] *** warning: No data (or metadata) defined in specification tree
[13:40:55][PDI] *** info: Initialization successful
Hello world!
[13:40:55][PDI] *** info: Finalization
[13:40:55][User-code] *** info: Closing plugin
```

## Handling input {#handling_input_node}

Now we pass some input data to the function.

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

Output:
```console
[13:49:30][User-code] *** info: Plugin loaded successfully
[13:49:30][PDI] *** info: Initialization successful
I've got number 42.
[13:49:30][PDI] *** info: Finalization
[13:49:30][User-code] *** info: Closing plugin

```

We can simplify this example by using `on_data` to print the value of `number`
when it is shared with %PDI.

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

The output does not change:
```console
[13:52:15][User-code] *** info: Plugin loaded successfully
[13:52:15][PDI] *** info: Initialization successful
I've got number 42.
[13:52:15][PDI] *** info: Finalization
[13:52:15][User-code] *** info: Closing plugin
```

## Handling output {#handling_output_node}

Output handling is very similar to input handling, the only
difference being the access rights. In this example, we call the
`add_ten` function when `number` is shared with %PDI.
Since `add_ten` both reads and writes its input, it accesses it with `PDI_INOUT`.

adding_to_number.c:
```C
#include <stdio.h>
#include <pdi.h>

void add_ten(void)
{
    void* input; PDI_access("input", &input, PDI_INOUT);
    *((int*)input) += 10;
    PDI_release("input");
}

int main(int argc, char* argv[])
{
    PC_tree_t conf = PC_parse_path("adding_to_number.yml");
    PDI_init(conf);
    int number = 42;
    printf("Before expose, number = %d.\n", number);
    PDI_expose("number", &number, PDI_INOUT);
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

Output:
```console
[13:53:51][User-code] *** info: Plugin loaded successfully
[13:53:51][PDI] *** info: Initialization successful
Before expose, number = 42.
After expose, number = 52.
[13:53:51][PDI] *** info: Finalization
[13:53:51][User-code] *** info: Closing plugin

```

## Multiple input/output data {#multiple_inout_data_node}

In this example, we pass several data to the function. It adds
and multiplies two given numbers, and returns the results, on the "calculate" event.

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

Output:
```console
[13:58:20][User-code] *** info: Plugin loaded successfully
[13:58:20][PDI] *** info: Initialization successful
Before calculation, foo = 4, bar = 5, res1 = 0, res2 = 0.
After calculation, foo = 4, bar = 5, res1 = 9, res2 = 20.
[13:58:20][PDI] *** info: Finalization
[13:58:20][User-code] *** info: Closing plugin
```

## Calling a function on some iterations only {#every_other_node}

A function list can have a `when` condition: here, the function is only called at the even iterations.

every_other.c:
```C
#include <stdio.h>
#include <pdi.h>

void report(void)
{
    int* step;
    PDI_access("step", (void**)&step, PDI_IN);
    printf("Reporting at iteration %d.\n", *step);
    PDI_release("step");
}

int main(int argc, char* argv[])
{
    PC_tree_t conf = PC_parse_path("every_other.yml");
    PDI_init(conf);
    for (int iter = 0; iter < 5; ++iter) {
        PDI_expose("iter", &iter, PDI_OUT);
    }
    PDI_finalize();
    return 0;
}
```

every_other.yml:
```yaml
metadata:
    iter: int
plugins:
    user_code:
        on_data:
            iter:
                when: $iter % 2 = 0   # only at the even iterations
                report: {step: $iter}
```

Output:
```console
[14:05:23][User-code] *** info: Plugin loaded successfully
[14:05:23][PDI] *** info: Initialization successful
Reporting at iteration 0.
Reporting at iteration 2.
Reporting at iteration 4.
[14:05:23][PDI] *** info: Finalization
[14:05:23][User-code] *** info: Closing plugin
```
