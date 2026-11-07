# Serialize plugin {#serialize_plugin}

Serialize plugin allows to serialize shared data. The plugin will convert all
arrays from sparse to dense and dereference all pointers.

The serialized data will be shared as long the user data is shared.

1. In case of the share with `PDI_OUT`: plugin will serialize and share serialized data on `PDI_share` of given descriptor.
2. In case of the share with `PDI_IN`: plugin will deserialize data on `PDI_reclaim` of given descriptor
(to be sure that the serialized data have been writen to buffer by other plugin (e.g. done on event)).
3. In case of the share with `PDI_INOUT`: plugin will do step 1. on `PDI_share` and step 2. on `PDI_reclaim`.

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
On each `sparse_array` data share, the plugin will share serialized data under the `dense_array` name.
The `dense_array` will be of type:
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
On each `pointer_to_sparse_array` data share, the plugin will share serialized data under the `dense_array` name.
The `dense_array` again will be of type:
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


