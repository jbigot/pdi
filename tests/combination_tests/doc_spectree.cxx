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
/// The examples of the specification tree reference (pdi/docs/Specification_tree_ref.md).
/// They load the Trace, Decl'HDF5 and MPI plugins, so they are checked here rather than beside the reference itself.

#include <filesystem>
#include <fstream>
#include <regex>
#include <string>
#include <vector>

#include <mpi.h>

#include <gtest/gtest.h>
#include <paraconf.h>

#include <pdi.h>
#include <pdi/testing.h>

namespace {

/// The directory of the reference's examples, with the files they include
const std::filesystem::path SPECTREE_DIR = SPECIFICATION_TREE_REF_DIR;

/// Shares then reclaims each metadata and data that the tree declares, so that their datatype is evaluated and the plugins act on them.
/// Their content is zero: that is a valid value for each of them, a size of zero for those used as such.
void share_all(PC_tree_t tree)
{
	alignas(std::max_align_t) static char buffer[1 << 16] = {};
	for (auto&& section: {".metadata", ".data"}) {
		int nb_names = 0;
		PC_errhandler_t pc_handler = PC_errhandler(PC_NULL_HANDLER);
		PC_len(PC_get(tree, "%s", section), &nb_names);
		PC_errhandler(pc_handler);
		for (int name_id = 0; name_id < nb_names; ++name_id) {
			char* name = nullptr;
			ASSERT_EQ(PC_OK, PC_string(PC_get(tree, "%s{%d}", section, name_id), &name));
			EXPECT_EQ(PDI_OK, PDI_expose(name, buffer, PDI_OUT)) << name;
			free(name);
		}
	}
}

/// The lines of a labelled region of a snippet file
std::vector<std::string> snippet(const std::filesystem::path& file, const std::string& label)
{
	std::ifstream input(file);
	std::vector<std::string> result;
	bool inside = false;
	for (std::string line; std::getline(input, line);) {
		if (line == "#! [" + label + "]") {
			if (inside) return result;
			inside = true;
		} else if (inside) {
			result.push_back(line);
		}
	}
	ADD_FAILURE() << "no region " << label << " in " << file;
	return result;
}

/// A regular expression that matches the lines an spdlog pattern produces, in a file
std::regex pattern_regex(std::string pattern)
{
	// the only %{} block of the documented patterns: the rank of the single process of the test
	pattern = std::regex_replace(pattern, std::regex(R"(%\{MPI_COMM_WORLD_rank:06d\})"), "000000");
	pattern = std::regex_replace(pattern, std::regex(R"([.^$|()\\[\]{}*+?])"), R"(\$&)");
	pattern = std::regex_replace(pattern, std::regex(R"(%T)"), "[0-9]{2}:[0-9]{2}:[0-9]{2}");
	pattern = std::regex_replace(pattern, std::regex(R"(%n)"), "[^\\]]+");
	pattern = std::regex_replace(pattern, std::regex(R"(%l)"), "(trace|debug|info|warning|error|critical)");
	pattern = std::regex_replace(pattern, std::regex(R"(%\\\^|%\\\$)"), ""); // the color range, empty in a file
	pattern = std::regex_replace(pattern, std::regex(R"(%v)"), ".*");
	return std::regex(pattern);
}

/// Checks that each line of a log file matches a pattern, from the first one that contains a text on
void expect_log_matches(const std::filesystem::path& log, const std::string& pattern, const std::string& from = "")
{
	std::regex expected = pattern_regex(pattern);
	std::ifstream input(log);
	int nb_lines = 0;
	for (std::string line; std::getline(input, line);) {
		if (nb_lines == 0 && line.find(from) == std::string::npos) continue;
		++nb_lines;
		EXPECT_TRUE(std::regex_match(line, expected)) << "`" << line << "' does not match `" << pattern << "'";
	}
	EXPECT_LT(0, nb_lines) << "nothing logged to " << log << " from `" << from << "' on";
}

struct DocSpectree: public ::PDI::PdiTest {};

/// Every example of the reference is a valid specification tree that PDI loads, the plugins it names included, and whose data PDI accepts.
TEST_F(DocSpectree, examples)
{
	// the examples include files by paths relative to the working directory
	std::filesystem::copy(SPECTREE_DIR, ".", std::filesystem::copy_options::recursive);
	PC_tree_t examples = PC_parse_path("spectree.yml");
	int nb_examples = 0;
	ASSERT_EQ(PC_OK, PC_len(examples, &nb_examples));
	for (int example_id = 0; example_id < nb_examples; ++example_id) {
		char* name = nullptr;
		ASSERT_EQ(PC_OK, PC_string(PC_get(examples, "{%d}", example_id), &name));
		SCOPED_TRACE(name);
		free(name);
		// FinalizePdi destroys the whole document the tree belongs to, so each example is parsed anew
		PC_tree_t example = PC_get(PC_parse_path("spectree.yml"), "<%d>", example_id);
		InitPdi(example);
		share_all(example);
		FinalizePdi();
	}
	PC_tree_destroy(&examples);
}

/// The default pattern of the loggers is the documented one, without the MPI plugin.
TEST_F(DocSpectree, default_pattern)
{
	InitPdi(PC_parse_string("logging: {level: debug, output: {file: pdi.log}}"));
	FinalizePdi();
	for (auto&& line: snippet(SPECTREE_DIR / "default_patterns.txt", "serial")) {
		expect_log_matches("pdi.log", line);
	}
}

/// The default pattern of the loggers is the documented one once the MPI plugin is loaded, for PDI and for the plugins.
TEST_F(DocSpectree, default_pattern_mpi)
{
	InitPdi(PC_parse_string(R"==(
logging: {level: debug, output: {file: pdi.log}}
plugins:
  mpi: {logging: {output: {file: mpi.log}}}
)=="));
	FinalizePdi();
	for (auto&& line: snippet(SPECTREE_DIR / "default_patterns.txt", "mpi")) {
		expect_log_matches("pdi.log", line, "Initialization successful");
		expect_log_matches("mpi.log", line);
	}
}

} // namespace

/// The MPI plugin needs MPI to be initialized
int main(int argc, char** argv)
{
	::testing::InitGoogleTest(&argc, argv);
	MPI_Init(&argc, &argv);
	int result = RUN_ALL_TESTS();
	MPI_Finalize();
	return result;
}
