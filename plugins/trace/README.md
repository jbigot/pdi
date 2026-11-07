# Trace plugin {#trace_plugin}

The Trace plugin generates a trace of what happens in the
\ref Data_store "PDI data store".

The plugin is very simple: just list its name in the
\ref Specification_tree "specification tree" and you're done.

```yaml
data: #...
metadata: #...
plugins:
  trace:
  #...
```

The plugin then generates a (somewhat verbose) trace through its logger, on the
standard output by default.

For example:
```
[10:52:58][Trace] *** info: Welcome!
[10:52:58][PDI] *** info: Initialization successful
[10:52:58][Trace] *** info: !!!                            named event: pdi_start_timer
[10:52:58][Trace] *** info: =>>   data becoming available in the store: my_data
[10:52:58][Trace] *** info: <<= data stop being available in the store: my_data
[10:52:58][Trace] *** info: !!!                            named event: my_event
[10:52:58][Trace] *** info: !!!                            named event: pdi_stop_timer
[10:52:58][PDI] *** info: Finalization
[10:52:58][Trace] *** info: Goodbye!
```

Each line starting with `=>>   data becoming available in the store` means a new
piece of data has been shared.

Each line starting with `<<= data stop being available in the store` means a piece
of data is being removed from the store.

Each line starting with `!!!                            named event` means a named
event has been emitted.

## `@trace` {#trace_config}

Empty (`~`), or a mapping whose only key is \ref trace_logging.
Use it, as the value of `plugins.trace` (see \ref spectree_pdi_plugins_plugin_name), to load and configure the Trace plugin.

**Example:**

```yaml
plugins:
  trace: ~                # <- no configuration: the trace goes to the standard output
```


### `@trace.logging` {#trace_logging}

*Optional.*
A \ref spectree_logging.
Use `logging` to configure the logger of the plugin, through which it writes the trace, see \ref spectree_pdi_plugins_plugin_name_logging.

**Example:**

```yaml
logging:
  level: info
  output:
    file: trace.log   # <- the trace goes to trace.log instead of the standard output
```
