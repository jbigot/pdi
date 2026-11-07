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
/// Runs the specification trees of the Set_value examples page
/// (plugins/set_value/examples.md) with the programs it describes, and checks
/// what they get against what the page says.
/// The examples that load other plugins are run in tests/combination_tests.

#include <array>
#include <cstdio>
#include <string>

#include <gtest/gtest.h>

#include <pdi.h>

namespace {

/// The path of the specification tree of an example
std::string tree(const std::string& name)
{
	return std::string(EXAMPLES_DIR) + "/" + name + ".yml";
}

/// The program of the "Computing new values from old ones" examples: it emits init and increment, then reads int_scalar and int_array
void values(const std::string& name, int scalar, std::array<int, 3> array)
{
	ASSERT_EQ(PDI_init(PC_parse_path(tree(name).c_str())), PDI_OK);
	ASSERT_EQ(PDI_event("init"), PDI_OK);
	ASSERT_EQ(PDI_event("increment"), PDI_OK);
	int* value;
	ASSERT_EQ(PDI_access("int_scalar", (void**)&value, PDI_IN), PDI_OK);
	EXPECT_EQ(*value, scalar);
	ASSERT_EQ(PDI_release("int_scalar"), PDI_OK);
	int* values;
	ASSERT_EQ(PDI_access("int_array", (void**)&values, PDI_IN), PDI_OK);
	EXPECT_EQ((std::array<int, 3>{values[0], values[1], values[2]}), array);
	ASSERT_EQ(PDI_release("int_array"), PDI_OK);
	ASSERT_EQ(PDI_finalize(), PDI_OK);
}

} // namespace

TEST(SetValueExamples, params)
{
	ASSERT_EQ(PDI_init(PC_parse_path(tree("params").c_str())), PDI_OK);
	int* nb_iter;
	double* alpha;
	ASSERT_EQ(PDI_access("nb_iter", (void**)&nb_iter, PDI_IN), PDI_OK);
	ASSERT_EQ(PDI_access("alpha", (void**)&alpha, PDI_IN), PDI_OK);
	EXPECT_EQ(*nb_iter, 10);
	EXPECT_EQ(*alpha, 0.125);
	ASSERT_EQ(PDI_release("alpha"), PDI_OK);
	ASSERT_EQ(PDI_release("nb_iter"), PDI_OK);
	ASSERT_EQ(PDI_finalize(), PDI_OK);
}

TEST(SetValueExamples, record)
{
	struct params {
		int nb_iter;
		double alpha;
		int size[2];
	};

	ASSERT_EQ(PDI_init(PC_parse_path(tree("record").c_str())), PDI_OK);
	params* p;
	ASSERT_EQ(PDI_access("params", (void**)&p, PDI_IN), PDI_OK);
	EXPECT_EQ(p->nb_iter, 10);
	EXPECT_EQ(p->alpha, 0.125);
	EXPECT_EQ(p->size[0], 60);
	EXPECT_EQ(p->size[1], 12);
	ASSERT_EQ(PDI_release("params"), PDI_OK);
	ASSERT_EQ(PDI_finalize(), PDI_OK);
}

TEST(SetValueExamples, counter)
{
	ASSERT_EQ(PDI_init(PC_parse_path(tree("counter").c_str())), PDI_OK);
	for (int iter = 0; iter < 3; ++iter) {
		ASSERT_EQ(PDI_event("step"), PDI_OK);
		int* counter;
		ASSERT_EQ(PDI_access("counter", (void**)&counter, PDI_IN), PDI_OK);
		EXPECT_EQ(*counter, iter + 1);
		ASSERT_EQ(PDI_release("counter"), PDI_OK);
	}
	ASSERT_EQ(PDI_finalize(), PDI_OK);
}

TEST(SetValueExamples, increment)
{
	values("increment", 1, {2, 3, 4});
}

TEST(SetValueExamples, old_value)
{
	values("old_value", 0, {1, 1, 1});
}

TEST(SetValueExamples, new_value)
{
	values("new_value", 1, {1, 1, 1});
}

TEST(SetValueExamples, level)
{
	// the program of the examples that configure other plugins: it shares iter, then field, at each of 2 iterations
	::testing::internal::CaptureStdout();
	ASSERT_EQ(PDI_init(PC_parse_path(tree("level").c_str())), PDI_OK);
	double field[3];
	for (int iter = 0; iter < 2; ++iter) {
		for (int i = 0; i < 3; ++i)
			field[i] = 10 * iter + i;
		ASSERT_EQ(PDI_expose("iter", &iter, PDI_OUT), PDI_OK);
		ASSERT_EQ(PDI_expose("field", field, PDI_OUT), PDI_OK);
	}
	ASSERT_EQ(PDI_finalize(), PDI_OK);
	fflush(stdout);
	std::string log = ::testing::internal::GetCapturedStdout();

	// trace messages between the share of iter and that of field, and none out of them
	std::string to_trace = "[Set-value] warning: Changing level to trace\n";
	std::string to_info = "[Set-value] warning: Changing level to info\n";
	size_t start = 0;
	for (int iter = 0; iter < 2; ++iter) {
		size_t trace = log.find(to_trace, start);
		ASSERT_NE(trace, std::string::npos) << log;
		size_t info = log.find(to_info, trace);
		ASSERT_NE(info, std::string::npos) << log;
		EXPECT_EQ(log.substr(start, trace - start).find("] trace: "), std::string::npos) << log;
		EXPECT_NE(log.substr(trace, info - trace).find("[PDI] trace: Sharing `field'"), std::string::npos) << log;
		start = info + to_info.size();
	}
	EXPECT_EQ(log.substr(start).find("] trace: "), std::string::npos) << log;
}
