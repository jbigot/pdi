# Serialize plugin {#serialize_plugin}

The Serialize plugin serializes shared data: it converts all sparse arrays to dense
ones, and dereferences all pointers.

The serialized data is shared as long as the user data is shared.

1. When the data is shared with `PDI_OUT`, the plugin serializes it and shares the serialized data on `PDI_share`.
2. When the data is shared with `PDI_IN`, the plugin deserializes it on `PDI_reclaim`
(so that another plugin has had the time to write the serialized data, on an event for example).
3. When the data is shared with `PDI_INOUT`, the plugin does both: step 1 on `PDI_share`, and step 2 on `PDI_reclaim`.

## Plugin examples {#serialize_plugin_examples}

```yaml
data:
  sparse_array:
    type: array
    subtype: int
    size: 8
    start: 2
    subsize: 4
plugins:
  serialize:
    logging: debug
    sparse_array: dense_array
```
Each time `sparse_array` is shared, the plugin shares its serialized version under the name `dense_array`, of type:
```yaml
type: array
subtype: int
size: 4
```

Another example:
```yaml
data:
  pointer_to_sparse_array:
    type: pointer
    subtype:
      type: array
      subtype: int
      size: 8
      start: 2
      subsize: 4
plugins:
  serialize:
    pointer_to_sparse_array: dense_array
```
Each time `pointer_to_sparse_array` is shared, the plugin shares its serialized version under the name `dense_array`, again of type:
```yaml
type: array
subtype: int
size: 4
```

## `@serialize` {#serialize_config}

A mapping with the keys below.
Use it, as the value of `plugins.serialize` (see \ref spectree_pdi_plugins_plugin_name), to name the serialized version of each data to serialize.

**Example:**

```yaml
plugins:
  serialize:              # <- the serialized version of each data, and its name
    sparse_array: dense_array
```


### `@serialize.<data_name>` {#serialize_data_name}

A scalar.
Use `<data_name>` to share, under this name, the serialized version of the data named `<data_name>`.

**Example:**

```yaml
sparse_array: dense_array # <- sparse_array is also shared as dense_array, serialized
```


### `@serialize.logging` {#serialize_logging}

*Optional.*
A \ref spectree_logging.
Use `logging` to configure the logger of the plugin, see \ref spectree_pdi_plugins_plugin_name_logging.
This plugin only supports its scalar form: a level.

**Example:**

```yaml
logging: debug            # <- a level only
sparse_array: dense_array
```


