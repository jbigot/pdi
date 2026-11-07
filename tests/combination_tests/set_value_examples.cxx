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
/// (plugins/set_value/examples.md) that load other plugins, with the program it
/// describes, and checks what they do against what the page says.

#include <cstdio>
#include <filesystem>
#include <string>

#include <stdlib.h>

#include <gtest/gtest.h>

#include <pdi.h>

namespace {

/// The path of the specification tree of an example
std::string tree(const std::string& name)
{
	return std::string(SET_VALUE_EXAMPLES_DIR) + "/" + name + ".yml";
}

/// The program of the examples that configure other plugins: it shares iter, then field, at each of 2 iterations
void loop(const std::string& name)
{
	ASSERT_EQ(PDI_init(PC_parse_path(tree(name).c_str())), PDI_OK);
	double field[3];
	for (int iter = 0; iter < 2; ++iter) {
		for (int i = 0; i < 3; ++i)
			field[i] = 10 * iter + i;
		ASSERT_EQ(PDI_expose("iter", &iter, PDI_OUT), PDI_OK);
		ASSERT_EQ(PDI_expose("field", field, PDI_OUT), PDI_OK);
	}
	ASSERT_EQ(PDI_finalize(), PDI_OK);
}

/// Each example runs in a directory of its own, removed afterwards
class SetValueExamples: public ::testing::Test
{
	std::filesystem::path m_previous;
	std::filesystem::path m_dir;

protected:
	void SetUp() override
	{
		m_previous = std::filesystem::current_path();
		std::string dir = "set_value_examples-XXXXXX";
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

#ifdef EXAMPLES_DECL_HDF5
TEST_F(SetValueExamples, events)
{
	loop("events");

	// read the datasets back
	ASSERT_EQ(
		PDI_init(PC_parse_string(R"(
data:
  field_0: {type: array, subtype: double, size: 3}
  field_1: {type: array, subtype: double, size: 3}
plugins:
  decl_hdf5:
    file: output.h5
    read: [field_0, field_1]
)")),
		PDI_OK
	);
	double field_0[3] = {}, field_1[3] = {};
	ASSERT_EQ(PDI_expose("field_0", field_0, PDI_IN), PDI_OK);
	ASSERT_EQ(PDI_expose("field_1", field_1, PDI_IN), PDI_OK);
	ASSERT_EQ(PDI_finalize(), PDI_OK);
	for (int i = 0; i < 3; ++i) {
		EXPECT_EQ(field_0[i], i);
		EXPECT_EQ(field_1[i], 10 + i);
	}
}
#endif

#ifdef EXAMPLES_TRACE
TEST_F(SetValueExamples, logger)
{
	::testing::internal::CaptureStdout();
	loop("logger");
	fflush(stdout);
	std::string log = ::testing::internal::GetCapturedStdout();
	EXPECT_NE(log.find("\n[Trace] Welcome!\n"), std::string::npos) << log;
	EXPECT_NE(log.find("\n[iteration 0][Trace] =>>   data becoming available in the store: field\n"), std::string::npos) << log;
	EXPECT_NE(log.find("\n[iteration 1][Trace] =>>   data becoming available in the store: field\n"), std::string::npos) << log;
}
#endif
