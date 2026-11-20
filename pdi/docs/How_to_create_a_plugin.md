# How to create a plugin {#how_to_create_a_plugin}

This tutorial shows how to write the code of a plugin.
Before designing your own, read the \ref plugin_design_guidelines "plugin design guidelines", which describe how a plugin should fit the
declarative approach of %PDI.

## Plugin class {#plugin_class}

Plugin is a class, that handles shared data and triggered events to perform an I/O operations. It is dynamically linked to the user program by PDI. Behavior of each plugin is defined by specification tree in plugins subtree.

```yaml
plugins:
  example:
    ...
```
The given example will load example plugin and pass its subtree to the plugin's constructor.

The plugin has to inherit from PDI::Plugin and have a constructor with arguments
PDI::Logger&, PDI::Context& and PC_tree_t.
It forwards the logger and the context to PDI::Plugin.

### Example plugin {#example_plugin}
Example of the simplest plugin, that does nothing:

```cpp
#include <pdi/context.h>
#include <pdi/logger.h>
#include <pdi/paraconf_wrapper.h>
#include <pdi/plugin.h>

struct example_plugin : public PDI::Plugin
{
    example_plugin(PDI::Logger& logger, PDI::Context& ctx, PC_tree_t spec_tree):
        Plugin{logger, ctx}
    {}
};

PDI_PLUGIN(example)
```

### Adding a callback {#adding_callback}
To be notified when the user shares data (calls `PDI_share`, `PDI_expose` or `PDI_multiexpose`), a plugin registers a function (a callback) with `PDI::Context::on_data`.

```cpp
std::function<void()> on_data(const std::function<void(const std::string&, Ref)>& callback, const std::string& name = {})
```

The first argument is the function to be called when user shares the data.
If the second parameter (name) is given, the function will be called only on a specified data name.
Returns a function that removes the callback from PDI::Context container.

Example of adding new callback:
```cpp
#include <iostream>

#include <pdi/context.h>
#include <pdi/logger.h>
#include <pdi/paraconf_wrapper.h>
#include <pdi/plugin.h>
#include <pdi/ref_any.h>

struct example_plugin : public PDI::Plugin
{
    example_plugin(PDI::Logger& logger, PDI::Context& ctx, PC_tree_t spec_tree):
        Plugin{logger, ctx}
    {
        ctx.on_data([](const std::string& data_name, PDI::Ref ref){
            std::cout << "User has shared a data named " << data_name << std::endl;
        });
    }
};

PDI_PLUGIN(example)
```
If user create specification tree:
```yaml
data:
    some_integer: int

plugins:
    example: ~
```
And a program:
```cpp
#include <pdi.h>

int main()
{
    PDI_init(PC_parse_path("spec_tree.yaml"));
    int some_integer = 0;
    PDI_expose("some_integer", &some_integer, PDI_OUT);
    PDI_finalize();
    return 0;
}
```
The console will display:
```
[PDI][13:42:41] *** info: Initialization successful
User has shared a data named some_integer
[PDI][13:42:42] *** info: Finalization
```

### Reading and writing data {#rw_data}

A `PDI::Ref` gives no access to the data by itself.
The plugin converts it to a `PDI::Ref_r` to read the data, a `PDI::Ref_w` to write it, or a `PDI::Ref_rw` to do both, and the converted
reference is null if the share does not grant that access.
Which access to ask for follows from what the plugin was configured to do, never from what the share happens to grant: see
\ref pdg_config_decides "the design guidelines".

The type of the reference describes the data: check it before interpreting the buffer.
Example of a plugin that reads an integer each time one is shared:
```cpp
#include <iostream>

#include <pdi/context.h>
#include <pdi/error.h>
#include <pdi/logger.h>
#include <pdi/paraconf_wrapper.h>
#include <pdi/plugin.h>
#include <pdi/ref_any.h>

struct example_plugin : public PDI::Plugin
{
    example_plugin(PDI::Logger& logger, PDI::Context& ctx, PC_tree_t spec_tree):
        Plugin{logger, ctx}
    {
        ctx.on_data([](const std::string& data_name, PDI::Ref ref){
            // this plugin reads the data: ask for read access
            if (PDI::Ref_r ref_r{ref}) {
                // converts the value to long, whatever the type of integer, and throws if it is not a scalar
                long some_integer = ref_r.scalar_value<long>();
                std::cout << data_name << " = " << some_integer << std::endl;
            } else {
                throw PDI::Permission_error{"Cannot read `{}'", data_name};
            }
        });
    }
};

PDI_PLUGIN(example)
```

### Handling events {#example_events}

To be notified of events, a plugin registers a callback with `PDI::Context::on_event`, either for all events, or for one event given by name.
Like the names of data, the names of events are chosen by the user, in the specification tree:
```yaml
plugins:
  example:
    on_event: checkpoint
```
```cpp
#include <iostream>
#include <string>

#include <pdi/context.h>
#include <pdi/logger.h>
#include <pdi/paraconf_wrapper.h>
#include <pdi/plugin.h>
#include <pdi/ref_any.h>

struct example_plugin : public PDI::Plugin
{
    example_plugin(PDI::Logger& logger, PDI::Context& ctx, PC_tree_t spec_tree):
        Plugin{logger, ctx}
    {
        ctx.on_event([this](const std::string& event_name){
            this->handle_event(event_name);
        });
        ctx.on_event([this](const std::string& event_name){
            this->handle_configured_event(event_name);
        }, PDI::to_string(PC_get(spec_tree, ".on_event")));
    }
private:
    void handle_event(const std::string& event_name) {
        std::cout << "Event `" << event_name << "' called." << std::endl;
    }

    void handle_configured_event(const std::string& event_name) {
        std::cout << "Configured event `" << event_name << "' called." << std::endl;
    }
};

PDI_PLUGIN(example)
```

### Reading scalar and array from specification tree {#reading_pc_tree}
Specification tree:
```yaml
plugins:
  example:
    scalar: some_string
    array: [0, 1, 2]
```
Reading a scalar and an array:
```cpp
#include <string>
#include <vector>

#include <pdi/context.h>
#include <pdi/logger.h>
#include <pdi/paraconf_wrapper.h>
#include <pdi/plugin.h>
#include <pdi/ref_any.h>

struct example_plugin : public PDI::Plugin
{
    example_plugin(PDI::Logger& logger, PDI::Context& ctx, PC_tree_t spec_tree):
        Plugin{logger, ctx}
    {
        //scalar
        PC_tree_t scalar_tree = PC_get(spec_tree, ".scalar");
        std::string string_scalar = PDI::to_string(scalar_tree);

        //array
        PC_tree_t array_tree = PC_get(spec_tree, ".array");
        int array_size = PDI::len(array_tree);
        std::vector<long> array;
        for (int i = 0; i < array_size; i++) {
            PC_tree_t array_element = PC_get(array_tree, "[%d]", i);
            array.emplace_back(PDI::to_long(array_element));
        }
    }
};

PDI_PLUGIN(example)
```
### Reading maps from specification tree {#reading_pc_tree_2}
Specification tree:
```yaml
plugins:
  example:
    custom_subtree:
        here: 0
        can: 1
        be: 2
        any: 3
        subtree: 4
```
Reading a map:
```cpp
#include <string>
#include <unordered_map>

#include <pdi/context.h>
#include <pdi/logger.h>
#include <pdi/paraconf_wrapper.h>
#include <pdi/plugin.h>
#include <pdi/ref_any.h>

struct example_plugin : public PDI::Plugin
{
    example_plugin(PDI::Logger& logger, PDI::Context& ctx, PC_tree_t spec_tree):
        Plugin{logger, ctx}
    {
        PC_tree_t subtree = PC_get(spec_tree, ".custom_subtree");
        int subtree_size = PDI::len(subtree);
        std::unordered_map<std::string, long> custom_map;

        for (int i = 0; i < subtree_size; i++) {
            PC_tree_t key = PC_get(subtree, "{%d}", i);
            PC_tree_t value = PC_get(subtree, "<%d>", i);
            custom_map.emplace(PDI::to_string(key), PDI::to_long(value));
        }
    }
};

PDI_PLUGIN(example)
```

## Creating a true plugin: POSIX plugin {#create_true_plugin}

The fastest way to learn is by example.
This section builds a "POSIX plugin" that writes data to files and reads it back, a minimal checkpointing plugin.
It does nothing a real I/O plugin would not do better, but it follows the \ref plugin_design_guidelines "plugin design guidelines", and
shows how each of them translates into code.

### Step 1: Think what your plugin will be for {#step_1}

Simple checkpointing: each data is saved to a file of its own, and read back from it on restart.
The application can also learn whether a checkpoint exists, to decide whether to restart.

The library here is POSIX file I/O, whose only concept is the file: the configuration describes files, which data to write to them or read
from them, and when.

### Step 2: Prepare your specification tree schema {#step_2}

```yaml
metadata:
  rank: int
  iter: int
  restart: int
data:
  field: {type: array, subtype: double, size: [64, 64]}
  checkpoint_found: int

plugins:
  posix:
    write:                          # data to write, each with where and when
      field:
        file: field_${rank}.bin     # a $-expression, evaluated on each write
        when: '$iter % 10 = 0'      # the configuration decides when to write
    read:                           # data to read, same form
      field:
        file: field_${rank}.bin
        when: $restart              # the configuration decides when to read
    exists:                         # data to fill with whether a file exists
      checkpoint_found: field_${rank}.bin
```

Each entry of `write` and `read` is a `FILE_OP`: either the name of the file alone, which always triggers, or a map with the `file` and a
`when` condition, which defaults to always.
`write: {field: field.bin}` is enough to write `field` to `field.bin` each time it is shared.

The application only shares `rank`, `iter`, `restart`, `field` and `checkpoint_found`.
It does not know which plugin uses them, and the access it grants on `field` (`PDI_OUT`, `PDI_IN` or `PDI_INOUT`) does not change what the
plugin does: the `when` conditions do.
The plugin only checks that the access is enough for what it was asked.

### Step 3: Write your plugin {#step_3}

Parse a `FILE_OP`, accepting the short form, and rejecting unknown keys with their location in the specification tree:
```cpp
#include <filesystem>
#include <fstream>
#include <memory>
#include <string>
#include <unordered_map>

#include <pdi/context.h>
#include <pdi/error.h>
#include <pdi/expression.h>
#include <pdi/logger.h>
#include <pdi/paraconf_wrapper.h>
#include <pdi/plugin.h>
#include <pdi/ref_any.h>

namespace {

/// A file to access, and the condition to access it
struct File_op {
    PDI::Expression file;
    PDI::Expression when = 1L;
};

/// Parses a FILE_OP: either the file name alone, or a map with `file' and optionally `when'
File_op parse_file_op(PC_tree_t tree)
{
    if (PDI::is_scalar(tree)) {
        return {PDI::to_string(tree)};
    }
    File_op result;
    PDI::each(tree, [&](PC_tree_t key_tree, PC_tree_t value) {
        std::string key = PDI::to_string(key_tree);
        if (key == "file") {
            result.file = PDI::to_string(value);
        } else if (key == "when") {
            result.when = PDI::to_string(value);
        } else {
            throw PDI::Spectree_error{key_tree, "Unknown key in POSIX file operation: `{}'", key};
        }
    });
    if (PC_status(PC_get(tree, ".file"))) {
        throw PDI::Spectree_error{tree, "Missing `file' in POSIX file operation"};
    }
    return result;
}

} // namespace
```

Store the operations to execute, indexed by the name of the data that triggers them, and register a single callback for all data.
The whole configuration is parsed and checked in the constructor, so that a mistake is reported when the plugin loads.
The $-expressions, on the other hand, are kept as `PDI::Expression`s, to be evaluated when they are used:
```cpp
class posix_plugin: public PDI::Plugin
{
    /// data name -> where and when to write it
    std::unordered_map<std::string, File_op> m_writes;

    /// data name -> where and when to read it
    std::unordered_map<std::string, File_op> m_reads;

    /// data name -> the file whose existence to store in it
    std::unordered_map<std::string, PDI::Expression> m_exists;

public:
    posix_plugin(PDI::Logger& logger, PDI::Context& ctx, PC_tree_t spec_tree)
        : Plugin{logger, ctx}
    {
        PDI::each(spec_tree, [&](PC_tree_t key_tree, PC_tree_t value) {
            std::string key = PDI::to_string(key_tree);
            if (key == "write") {
                PDI::each(value, [&](PC_tree_t data, PC_tree_t op) { m_writes.emplace(PDI::to_string(data), parse_file_op(op)); });
            } else if (key == "read") {
                PDI::each(value, [&](PC_tree_t data, PC_tree_t op) { m_reads.emplace(PDI::to_string(data), parse_file_op(op)); });
            } else if (key == "exists") {
                PDI::each(value, [&](PC_tree_t data, PC_tree_t file) { m_exists.emplace(PDI::to_string(data), PDI::to_string(file)); });
            } else if (key != "logging") { // `logging' is handled by PDI
                throw PDI::Spectree_error{key_tree, "Unknown key in POSIX plugin configuration: `{}'", key};
            }
        });
        ctx.on_data([this](const std::string& name, PDI::Ref ref) { this->data(name, ref); });
    }

private:
```

On each share, execute the operations whose `when` holds:
```cpp
    void data(const std::string& name, PDI::Ref ref)
    {
        // read before writing, so that a data both read and written on the same share is not
        // overwritten before it is read
        if (auto&& op = m_reads.find(name); op != m_reads.end() && op->second.when.to_long(context())) {
            read(name, ref, op->second.file.to_string(context()));
        }
        if (auto&& op = m_writes.find(name); op != m_writes.end() && op->second.when.to_long(context())) {
            write(name, ref, op->second.file.to_string(context()));
        }
        if (auto&& file = m_exists.find(name); file != m_exists.end()) {
            exists(name, ref, file->second.to_string(context()));
        }
    }
```

Write the data, asking for read access to it, and reporting every failure.
The data can be sparse in memory (a subset of a larger array, for example): its type describes its layout, and is used to make a dense copy:
```cpp
    void write(const std::string& name, PDI::Ref_r ref, const std::string& path)
    {
        if (!ref) {
            throw PDI::Permission_error{"Cannot read `{}' to write it to `{}'", name, path};
        }
        auto&& type = ref.type();
        const char* data = static_cast<const char*>(ref.get());
        std::unique_ptr<char[]> dense_copy;
        if (type->buffersize() != type->datasize()) {
            // the data is sparse in memory, make a dense copy of it
            dense_copy.reset(new char[type->datasize()]);
            type->data_to_dense_copy(dense_copy.get(), data);
            data = dense_copy.get();
        }

        // write to a temporary file, then replace the old one, so that a failed write keeps it
        std::string tmp_path = path + ".tmp";
        std::ofstream file{tmp_path, std::ios::binary};
        file.write(data, type->datasize());
        file.close();
        if (!file) {
            throw PDI::System_error{"Cannot write `{}' to `{}'", name, tmp_path};
        }
        std::error_code error;
        std::filesystem::rename(tmp_path, path, error);
        if (error) {
            throw PDI::System_error{"Cannot rename `{}' to `{}': {}", tmp_path, path, error.message()};
        }
    }
```

Read the data, asking for write access to it, and checking that the file holds the expected amount of data before copying it:
```cpp
    void read(const std::string& name, PDI::Ref_w ref, const std::string& path)
    {
        if (!ref) {
            throw PDI::Permission_error{"Cannot write `{}' to read it from `{}'", name, path};
        }
        auto&& type = ref.type();
        std::error_code error;
        auto file_size = std::filesystem::file_size(path, error);
        if (error) {
            throw PDI::System_error{"Cannot read `{}' from `{}': {}", name, path, error.message()};
        }
        if (file_size != type->datasize()) {
            throw PDI::Value_error{"`{}' holds {} B, `{}' needs {} B", path, file_size, name, type->datasize()};
        }

        std::unique_ptr<char[]> dense_copy{new char[type->datasize()]};
        std::ifstream file{path, std::ios::binary};
        file.read(dense_copy.get(), type->datasize());
        if (!file) {
            throw PDI::System_error{"Cannot read `{}' from `{}'", name, path};
        }
        // the data may be sparse in memory, copy it to its layout
        type->data_from_dense_copy(ref.get(), dense_copy.get());
    }
```

Store whether the file exists in the data the user chose.
`scalar_assign` writes the value according to the type of the data, whatever the size of the integer, and throws if it is not a scalar, so
that the plugin never writes past the end of the user's buffer:
```cpp
    void exists(const std::string& name, PDI::Ref_w ref, const std::string& path)
    {
        if (!ref) {
            throw PDI::Permission_error{"Cannot write to `{}' whether `{}' exists", name, path};
        }
        // assigns the value according to the type of the data, and throws if it is not a scalar
        ref.scalar_assign(std::filesystem::exists(path) ? 1 : 0);
    }
};

PDI_PLUGIN(posix)
```

The references stay alive for the whole duration of each callback, so that no other plugin accesses the data while it is being read or
written.

### Next steps {#plugin_compile}

1. Compile:
   `g++ -std=c++20 -shared -fPIC posix.cxx -o libpdi_posix_plugin.so -lpdi -lparaconf`
2. Copy the created file next to the other plugins, or to a directory listed in `PDI_PLUGIN_PATH`.
3. Write a program that shares the data of the specification tree above, compile it and run it.

The next step to make this plugin a real one would be to trigger operations on events as well as on shares, with an `on_event` key, as
\ref Decl_HDF5_plugin "Decl'HDF5" does.
