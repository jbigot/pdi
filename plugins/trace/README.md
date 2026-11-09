# Trace plugin {#trace_plugin}

The trace plugin is intended to generate a trace of what happens in 
\ref Data_store "PDI data store".

The plugin is very simple; just list its name in the 
\ref Specification_tree "specification tree" and you're done.

\snippet{trimleft} trace/docs/reference.yml trace_plugin

The plugin then generates a (somewhat verbose) trace through its logger, on the
standard output by default.

For example:
\snippet trace/docs/trace_output.output output

Each line starting by `=>>   data becoming available in the store` means a new
piece of data has been shared.

Each line starting by `<<= data stop being available in the store` means a piece
of data is being removed from the store.

Each line starting by `!!!                            named event` means a named
event has been emitted.

## `@trace` {#trace_config}

Empty (`~`), or a mapping whose only key is \ref trace_logging.
Use it, as the value of `plugins.trace` (see \ref spectree_pdi_plugins_plugin_name), to load and configure the Trace plugin.

**Example:**

\snippet{trimleft} trace/docs/reference.yml trace_config


### `@trace.logging` {#trace_logging}

*Optional.*
A \ref spectree_logging.
Use `logging` to configure the logger of the plugin, through which it writes the trace, see \ref spectree_pdi_plugins_plugin_name_logging.

**Example:**

\snippet{trimleft} trace/docs/reference.yml trace_logging
