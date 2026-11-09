# Timer plugin {#timer_plugin}

The timer plugin allows measuring time spent by PDI and its plugins.

Each timer records the time spent between a `start` and a `stop` event, accumulated over all the times it is started and stopped.
When the plugin is unloaded, it outputs the time accumulated by each timer that was started at least once.

## Example of a Timer configuration {#timer_configuration}

\snippet{trimleft} timer/docs/reference.yml timer_configuration

In this example, `timer_B` is equivalent to `timer_A`; `timer_C` records the time spent between `toto_start_timer` and `toto_stop_timer`, and
between `titi_start_timer` and `titi_stop_timer`; and `timer_E` records the time spent between `PDI_init` and `PDI_finalize`, including the time
spent in the simulation itself.

## `@timer` {#timer_config}

A mapping.
Use it, as the value of `plugins.timer` (see \ref spectree_pdi_plugins_plugin_name), to define timers, and where to output their results.
It can also be a sequence of single-key mappings, which has the same meaning.

**Example:**

\snippet{trimleft} timer/docs/reference.yml timer_config


**Example:**

\snippet{trimleft} timer/docs/reference.yml timer_config_sequence


### `@timer.<timer_name>` {#timer_timer_name}

Use `<timer_name>` to define a timer named `<timer_name>`, and the events that start and stop it, either:
* a mapping with both \ref timer_timer_name_start and \ref timer_timer_name_stop keys,
* a prefix, given as a scalar, or a sequence of prefixes: for each prefix, the timer is started by the `<prefix>_start_timer` event and stopped by the
  `<prefix>_stop_timer` event.

The prefix can be either a user-defined name, whose events the application has to emit, or one of the following:
* `pdi`, whose events %PDI emits in `PDI_init` and `PDI_finalize`,
* the name of a plugin that emits such events around its own work: `decl_hdf5`, `decl_netcdf`, `json`, `pycall`, `serialize` or `user_code`.

A timer started while already running, or stopped while not running, logs an error and is left unchanged.

**Example:**

\snippet{trimleft} timer/docs/reference.yml timer_timer_name


### `@timer.<timer_name>.start` {#timer_timer_name_start}

*Required in the mapping form.*
A scalar.
Use `start` to name the event that starts the timer.

**Example:**

\snippet{trimleft} timer/docs/reference.yml timer_timer_name_start


### `@timer.<timer_name>.stop` {#timer_timer_name_stop}

*Required in the mapping form.*
A scalar.
Use `stop` to name the event that stops the timer.

**Example:**

\snippet{trimleft} timer/docs/reference.yml timer_timer_name_stop


### `@timer.output_to` {#timer_output_to}

*Optional.*
A scalar: either `cout` (the default), for the standard output through the logger of the plugin, or the path of a file.
Use `output_to` to choose where to output the results of the timers.
As `cout` is reserved for the standard output, it cannot be used as the name of the output file.

The results are appended to the file, one line `<timer name>,<seconds>` per timer.
The plugin locks the file with `flock`, so that several processes can write to it one after the other.
If the underlying filesystem does not support `flock`, the results are not written, so the standard output is the safer choice.

**Example:**

\snippet{trimleft} timer/docs/reference.yml timer_output_to


### `@timer.logging` {#timer_logging}

*Optional.*
A \ref spectree_logging.
Use `logging` to configure the logger of the plugin, see \ref spectree_pdi_plugins_plugin_name_logging.
This plugin only supports its scalar form: a level.
Like any other key, it then also defines a timer, named `logging`, that uses the level as its prefix.

**Example:**

\snippet{trimleft} timer/docs/reference.yml timer_logging


