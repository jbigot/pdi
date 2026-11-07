# $-expressions {#spectree_expression}

A *$-expression* is a **scalar** whose content matches the following grammar:

\include docs/expression_grammar.in.txt

The rules for evaluating an expression are close to those of a shell such as Bash.

In addition to raw text, a `STRING_LITERAL` can contain references to the string value of some data in the store, as well as the result of an
operation enclosed in a dollar-parenthesis `$()`.
A backslash `\` escapes the character that follows it, whatever it is: `\$` stands for a dollar `$`, `\\` for a backslash `\`, and `\x` for `x`.
There is nothing to escape after the last character, so a string that ends with a lone backslash is an error.
YAML itself gives a meaning to `\` in a double-quoted scalar, so write `5\$`
there as `"5\\$"`, or use a plain or single-quoted scalar: `` '5\$' ``.

An `OPERATION` can combine terms with logical and arithmetic operators, grouped by parentheses.
The terms of an operation can be integer literals, floating-point literals, or references to the value of some data in the store.
An integer literal is hexadecimal if it starts with `0x`, octal if it starts with `0` (`010` is 8), and decimal otherwise.
A literal that contains a dot `.` is a floating-point literal.

A `REFERENCE` is introduced by a dollar `$` sign and optionally enclosed in curly braces `{`, `}`.
Its value is that of the data or metadata with the associated name.
It is good practice to reference values from the metadata section, as this prevents dangling references.
The value can be formatted with an {fmt} `format_spec` string, by adding a colon `:` followed by the `format_spec` just before the closing brace
`}` (see: https://fmt.dev/latest/syntax.html#grammar-token-format_spec).
A reference can designate the data itself, or a sub-element of it:
* an array element, with the square brackets `[`, `]` operator,
* a record member, with the dot `.` operator.

The type of the value of an `EXPRESSION` is as follows:
* a `REFERENCE` has the type of the referenced data in the store,
* an `OPERATION` is integer-valued, or floating-point-valued if one of its terms is,
* a `STRING_LITERAL` is string-valued.

Where a value is expected (and not only an integer or a string), a YAML **sequence** or **mapping** whose elements are $-expressions is accepted too:
* a **sequence** is array-valued if all its elements have the same type and tuple-valued otherwise,
* a **mapping** is record-valued, each key naming a member.

In addition, an integer can be interpreted as a string, or as a boolean where zero is false and any other value is true.

The following strings can also be interpreted as boolean values:
* true (1): `y`, `Y`, `yes`, `Yes`, `YES`, `true`, `True`, `TRUE`, `on`, `On`, `ON`.
* false (0): `n`, `N`, `no`, `No`, `NO`, `false`, `False`, `FALSE`, `off`, `Off`, `OFF`.

**Example:**

```yaml
'$my_data'
```

```yaml
'($my_data + 3) % 6'
```

```yaml
'${my_data.subarray[0]} * 42'
```

```yaml
'my name is ${my_name}'
```

```yaml
'${my_data:05d}'
'${my_data:b}'
'${my_data:1.5f}'
'${my_data:>15s}'
```
