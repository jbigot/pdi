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
 * * Neither the names of CEA, nor the names of the contributors may be used to
 *   endorse or promote products derived from this software without specific
 *   prior written  permission.
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
/// Runs the specification trees of the JSON examples page
/// (plugins/json/examples.md) with the program it describes, and checks the
/// files they write against what the page says of them.

#include <filesystem>
#include <fstream>
#include <string>

#include <stdlib.h>

#include <gtest/gtest.h>
#include <nlohmann/json.hpp>

#include <pdi.h>

namespace {

/// The path of the specification tree of an example
std::string tree(const std::string& name)
{
	return std::string(EXAMPLES_DIR) + "/" + name + ".yml";
}

/// The program that the page describes: it shares mesh, then step and temp at each of 3 steps
void run(const std::string& name)
{
	struct mesh {
		int dimensions[2];
		double spacings[2];
	};

	ASSERT_EQ(PDI_init(PC_parse_path(tree(name).c_str())), PDI_OK);
	mesh m = {{2, 3}, {0.5, 0.25}};
	ASSERT_EQ(PDI_expose("mesh", &m, PDI_OUT), PDI_OK);
	double temp[2][3];
	for (int step = 0; step < 3; ++step) {
		for (int i = 0; i < 2; ++i) {
			for (int j = 0; j < 3; ++j)
				temp[i][j] = 10 * step + 3 * i + j;
		}
		ASSERT_EQ(PDI_multi_expose("iteration", "step", &step, PDI_OUT, "temp", temp, PDI_OUT, NULL), PDI_OK);
	}
	ASSERT_EQ(PDI_finalize(), PDI_OK);
}

nlohmann::json read(const std::string& file)
{
	std::ifstream in(file);
	EXPECT_TRUE(in) << file;
	return nlohmann::json::parse(in);
}

/// Each example runs in a directory of its own, removed afterwards
class JsonExamples: public ::testing::Test
{
	std::filesystem::path m_previous;
	std::filesystem::path m_dir;

protected:
	void SetUp() override
	{
		m_previous = std::filesystem::current_path();
		std::string dir = "json_examples-XXXXXX";
		ASSERT_NE(mkdtemp(dir.data()), nullptr);
		m_dir = m_previous / dir;
		std::filesystem::current_path(m_dir);
	}

	void TearDown() override
	{
		std::filesystem::current_path(m_previous);
		std::filesystem::remove_all(m_dir);
	}
};

} // namespace

TEST_F(JsonExamples, one)
{
	run("one");
	EXPECT_EQ(read("step.json"), nlohmann::json::parse(R"([{"step": 0}, {"step": 1}, {"step": 2}])"));
}

TEST_F(JsonExamples, several)
{
	run("several");
	EXPECT_EQ(read("setup.json"), nlohmann::json::parse(R"([{"mesh": {"dimensions": [2, 3], "spacings": [0.5, 0.25]}}])"));
	EXPECT_EQ(read("steps.json"), nlohmann::json::parse(R"([
		{"step": 0},
		{"temp": [[0.0, 1.0, 2.0], [3.0, 4.0, 5.0]]},
		{"step": 2},
		{"temp": [[20.0, 21.0, 22.0], [23.0, 24.0, 25.0]]}
	])"));
}
