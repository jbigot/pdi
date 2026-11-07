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
/// Runs the specification trees of the User-code examples page
/// (plugins/user_code/examples.md) with the programs and the functions it
/// describes, and checks what they print and get against what the page says.

#include <cstdio>
#include <string>

#include <gtest/gtest.h>

#include <pdi.h>

// The functions of the examples, that the plugin calls: the executable exports their symbols
extern "C" {

void hello_world(void)
{
	printf("Hello world!\n");
}

void print_number(void)
{
	int* input;
	PDI_access("input", (void**)&input, PDI_IN);
	printf("I've got number %d.\n", *input);
	PDI_release("input");
}

void add_ten(void)
{
	int* input;
	PDI_access("input", (void**)&input, PDI_INOUT);
	*input += 10;
	PDI_release("input");
}

void sum_and_multiply(void)
{
	int *number1, *number2, *sum, *product;
	PDI_access("number1", (void**)&number1, PDI_IN);
	PDI_access("number2", (void**)&number2, PDI_IN);
	PDI_access("sum", (void**)&sum, PDI_OUT);
	PDI_access("product", (void**)&product, PDI_OUT);
	*sum = *number1 + *number2;
	*product = *number1 * *number2;
	PDI_release("number1");
	PDI_release("number2");
	PDI_release("sum");
	PDI_release("product");
}

void report(void)
{
	int* step;
	PDI_access("step", (void**)&step, PDI_IN);
	printf("Reporting at iteration %d.\n", *step);
	PDI_release("step");
}

} // extern "C"

namespace {

/// The path of the specification tree of an example
std::string tree(const std::string& name)
{
	return std::string(EXAMPLES_DIR) + "/" + name + ".yml";
}

/// What the program prints, but the log
std::string printed()
{
	fflush(stdout);
	std::string result;
	std::string out = ::testing::internal::GetCapturedStdout();
	for (size_t start = 0; start < out.size();) {
		size_t end = out.find('\n', start);
		if (end == std::string::npos) end = out.size() - 1;
		std::string line = out.substr(start, end + 1 - start);
		if (line[0] != '[') result += line;
		start = end + 1;
	}
	return result;
}

} // namespace

TEST(UserCodeExamples, hello_world)
{
	::testing::internal::CaptureStdout();
	ASSERT_EQ(PDI_init(PC_parse_path(tree("hello_world").c_str())), PDI_OK);
	ASSERT_EQ(PDI_event("print"), PDI_OK);
	ASSERT_EQ(PDI_finalize(), PDI_OK);
	EXPECT_EQ(printed(), "Hello world!\n");
}

TEST(UserCodeExamples, print_number_on_event)
{
	::testing::internal::CaptureStdout();
	ASSERT_EQ(PDI_init(PC_parse_path(tree("print_number_on_event").c_str())), PDI_OK);
	int number = 42;
	ASSERT_EQ(PDI_share("number", &number, PDI_OUT), PDI_OK);
	ASSERT_EQ(PDI_event("print"), PDI_OK);
	ASSERT_EQ(PDI_reclaim("number"), PDI_OK);
	ASSERT_EQ(PDI_finalize(), PDI_OK);
	EXPECT_EQ(printed(), "I've got number 42.\n");
}

TEST(UserCodeExamples, print_number_on_data)
{
	::testing::internal::CaptureStdout();
	ASSERT_EQ(PDI_init(PC_parse_path(tree("print_number_on_data").c_str())), PDI_OK);
	int number = 42;
	ASSERT_EQ(PDI_expose("number", &number, PDI_OUT), PDI_OK);
	ASSERT_EQ(PDI_finalize(), PDI_OK);
	EXPECT_EQ(printed(), "I've got number 42.\n");
}

TEST(UserCodeExamples, adding_to_number)
{
	ASSERT_EQ(PDI_init(PC_parse_path(tree("adding_to_number").c_str())), PDI_OK);
	int number = 42;
	ASSERT_EQ(PDI_expose("number", &number, PDI_INOUT), PDI_OK);
	ASSERT_EQ(PDI_finalize(), PDI_OK);
	EXPECT_EQ(number, 52);
}

TEST(UserCodeExamples, calculate)
{
	ASSERT_EQ(PDI_init(PC_parse_path(tree("calculate").c_str())), PDI_OK);
	int foo = 4, bar = 5, res1 = 0, res2 = 0;
	ASSERT_EQ(PDI_multi_expose("calculate", "foo", &foo, PDI_OUT, "bar", &bar, PDI_OUT, "res1", &res1, PDI_IN, "res2", &res2, PDI_IN, NULL), PDI_OK);
	ASSERT_EQ(PDI_finalize(), PDI_OK);
	EXPECT_EQ(res1, 9);
	EXPECT_EQ(res2, 20);
}

TEST(UserCodeExamples, every_other)
{
	::testing::internal::CaptureStdout();
	ASSERT_EQ(PDI_init(PC_parse_path(tree("every_other").c_str())), PDI_OK);
	for (int iter = 0; iter < 5; ++iter) {
		ASSERT_EQ(PDI_expose("iter", &iter, PDI_OUT), PDI_OK);
	}
	ASSERT_EQ(PDI_finalize(), PDI_OK);
	EXPECT_EQ(printed(), "Reporting at iteration 0.\nReporting at iteration 2.\nReporting at iteration 4.\n");
}
