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
/// The examples of the $-expressions reference (pdi/docs/Spectree_expressions.md), from Spectree_expressions/expressions.yml, evaluated
/// against the data that file declares, so that the value the reference implies is checked.

#include <array>
#include <cstdlib>
#include <string>

#include <gtest/gtest.h>
#include <paraconf.h>

#include <pdi.h>
#include <pdi/expression.h>
#include <pdi/testing.h>

#include "data_store.h"

namespace {

struct DocExpression: public ::PDI::PdiTest {};

/// Each example evaluates to the value expressions.yml gives it.
TEST_F(DocExpression, examples)
{
	std::string const path = PDI_DOCS_DIR "/Spectree_expressions/expressions.yml";
	InitPdi(PC_get(PC_parse_path(path.c_str()), ".pdi"));

	int my_data = 42;
	PDI_share("my_data", &my_data, PDI_OUT);
	double my_real = 3.5;
	PDI_share("my_real", &my_real, PDI_OUT);
	std::array<char, 3> my_name{'P', 'D', 'I'};
	PDI_share("my_name", my_name.data(), PDI_OUT);
	std::array<int, 4> my_record{2, 0, 0, 0};
	PDI_share("my_record", my_record.data(), PDI_OUT);

	PC_tree_t examples = PC_get(PC_parse_path(path.c_str()), ".examples");
	int nb_examples = 0;
	ASSERT_EQ(PC_OK, PC_len(examples, &nb_examples));
	for (int example_id = 0; example_id < nb_examples; ++example_id) {
		char* value = nullptr;
		ASSERT_EQ(PC_OK, PC_string(PC_get(examples, "[%d].value", example_id), &value));
		char* expression = nullptr;
		ASSERT_EQ(PC_OK, PC_string(PC_get(examples, "[%d].expression", example_id), &expression));
		EXPECT_EQ(value, PDI::Expression{PC_get(examples, "[%d].expression", example_id)}.to_string(PDI::Data_store::store())) << expression;
		free(expression);
		free(value);
	}
	PC_tree_destroy(&examples);

	PDI_reclaim("my_record");
	PDI_reclaim("my_name");
	PDI_reclaim("my_real");
	PDI_reclaim("my_data");
}

} // namespace
