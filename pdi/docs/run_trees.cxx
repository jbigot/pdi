/*******************************************************************************
 * Copyright (C) 2026 Commissariat a l'energie atomique et aux energies alternatives (CEA)
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 * * Redistributions of source code must retain the above copyright notice,
 *   this list of conditions and the following disclaimer.
 * * Redistributions in binary form must reproduce the above copyright notice,
 *   this list of conditions and the following disclaimer in the documentation
 *   and/or other materials provided with the distribution.
 * * Neither the name of CEA nor the names of its contributors may be used to
 *   endorse or promote products derived from this software without specific
 *   prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 ******************************************************************************/

/// \file
/// Runs a specification tree of a file of documentation examples.
///
/// usage: run_trees <trees.yml> <name> [--init-only] [--no-share] [--inout] [--value <metadata>=<int>]... [--event <event>]...
///
/// Each key at the root of `trees.yml` names a whole specification tree, of which the documentation shows a part.
/// PDI is initialized with the tree <name>, which checks that it is a valid specification tree, the plugins it loads included; then each of its
/// metadata is exposed and each of its data shared, in the order the tree declares them, from buffers of zeros, so that their datatypes are
/// evaluated and the plugins act on them; each event given with `--event` is then emitted, in order, before the data is reclaimed.
/// A metadata given with `--value` is exposed as an `int` of that value instead, for the sizes that zero would make empty.
/// The data is shared for reading only, or with `--inout` for reading and writing.
/// With `--no-share`, the events are emitted but nothing is shared, for the trees whose plugins share the data themselves.
/// With `--init-only`, the tree is only initialized, because sharing its data would make a plugin act on files or functions that the example
/// only names.
/// The tree runs in a directory named after it, emptied first, in the current directory, where it writes its files.
/// Built with `RUN_TREES_MPI` defined, MPI is initialized around the run, for the trees that use it.
///
/// Returns 0 if the tree ran without error.

#include <cstddef>
#include <cstdio>
#include <cstdlib>
#include <filesystem>
#include <map>
#include <string>
#include <vector>

#ifdef RUN_TREES_MPI
#include <mpi.h>
#endif

#include <paraconf.h>
#include <pdi.h>

namespace {

/// Whether a PDI error happened
bool g_failed = false;

/// Reports a PDI error and marks the tree as failed, instead of aborting
void record_error(PDI_status_t status, const char* message, void*)
{
	std::fprintf(stderr, "    PDI error %d: %s\n", static_cast<int>(status), message);
	g_failed = true;
}

/// The names in a mapping of a tree, an empty list when it is absent
std::vector<std::string> names(PC_tree_t tree, const char* key)
{
	std::vector<std::string> result;
	PC_errhandler_t pc_handler = PC_errhandler(PC_NULL_HANDLER);
	PC_tree_t mapping = PC_get(tree, "%s", key);
	int nb_names = 0;
	if (!PC_status(mapping) && !PC_len(mapping, &nb_names)) {
		for (int name_id = 0; name_id < nb_names; ++name_id) {
			char* name = nullptr;
			if (!PC_string(PC_get(mapping, "{%d}", name_id), &name)) {
				result.emplace_back(name);
				std::free(name);
			}
		}
	}
	PC_errhandler(pc_handler);
	return result;
}

/// A buffer of zeros, as large as any data of the examples
struct Zeros {
	alignas(std::max_align_t) char content[1 << 16] = {};
};

/// Runs a tree of the file, returns whether it succeeded
bool run_tree(
	const char* path,
	const char* tree_name,
	bool init_only,
	bool share,
	PDI_inout_t access,
	const std::map<std::string, int>& values,
	const std::vector<std::string>& events
)
{
	PC_tree_t tree = PC_get(PC_parse_path(path), ".%s", tree_name);
	if (PC_status(tree)) return false;
	if (PDI_init(tree)) return false;
	if (!init_only) {
		for (auto&& name: share ? names(tree, ".metadata") : std::vector<std::string>{}) {
			if (auto&& value = values.find(name); value != values.end()) {
				PDI_expose(name.c_str(), &value->second, PDI_OUT);
			} else {
				Zeros zeros;
				PDI_expose(name.c_str(), zeros.content, PDI_OUT);
			}
		}
		std::vector<std::string> data = share ? names(tree, ".data") : std::vector<std::string>{};
		std::vector<Zeros> buffers(data.size());
		for (std::size_t data_id = 0; data_id < data.size(); ++data_id) {
			PDI_share(data[data_id].c_str(), buffers[data_id].content, access);
		}
		for (auto&& event: events) {
			PDI_event(event.c_str());
		}
		for (auto&& name: data) {
			PDI_reclaim(name.c_str());
		}
	}
	PDI_finalize();
	return !g_failed;
}

} // namespace

int main(int argc, char* argv[])
{
#ifdef RUN_TREES_MPI
	MPI_Init(&argc, &argv);
#endif
	if (argc < 3) {
		std::fprintf(
			stderr,
			"usage: %s <trees.yml> <name> [--init-only] [--no-share] [--inout] [--value <metadata>=<int>]... [--event <event>]...\n",
			argv[0]
		);
		return 1;
	}
	const char* path = argv[1];
	const char* tree_name = argv[2];
	bool init_only = false;
	bool share = true;
	PDI_inout_t access = PDI_OUT;
	std::map<std::string, int> values;
	std::vector<std::string> events;
	for (int arg_id = 3; arg_id < argc; ++arg_id) {
		if (std::string(argv[arg_id]) == "--init-only") {
			init_only = true;
		} else if (std::string(argv[arg_id]) == "--no-share") {
			share = false;
		} else if (std::string(argv[arg_id]) == "--inout") {
			access = PDI_INOUT;
		} else if (std::string(argv[arg_id]) == "--value" && arg_id + 1 < argc) {
			std::string value = argv[++arg_id];
			values[value.substr(0, value.find('='))] = std::stoi(value.substr(value.find('=') + 1));
		} else if (std::string(argv[arg_id]) == "--event" && arg_id + 1 < argc) {
			events.emplace_back(argv[++arg_id]);
		} else {
			std::fprintf(stderr, "unexpected argument: %s\n", argv[arg_id]);
			return 1;
		}
	}

	// the tree reads its path relative to the current directory, which the tree is run from
	std::filesystem::path tree_path = std::filesystem::absolute(path);
	path = tree_path.c_str();
	std::filesystem::remove_all(tree_name);
	std::filesystem::create_directory(tree_name);
	std::filesystem::current_path(tree_name);

	PDI_errhandler({record_error, nullptr});
	bool success = run_tree(path, tree_name, init_only, share, access, values, events);
#ifdef RUN_TREES_MPI
	MPI_Finalize();
#endif
	return success ? 0 : 1;
}
