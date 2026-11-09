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
/// The examples of the datatypes reference (pdi/docs/Spectree_types.md), from Spectree_types/types.yml, and what the reference states of them.

#include <cstddef>
#include <cstdlib>
#include <string>

#include <gtest/gtest.h>
#include <paraconf.h>

#include <pdi.h>
#include <pdi/data_descriptor.h>
#include <pdi/datatype.h>
#include <pdi/ref_any.h>
#include <pdi/testing.h>

#include "data_store.h"

extern "C" {
#include "Spectree_types/particle.h"
}

namespace {

/// The path of the examples
std::string const TYPES_YML = PDI_DOCS_DIR "/Spectree_types/types.yml";

/// The tree of an example, in a document of its own
PC_tree_t example(const std::string& name)
{
	return PC_get(PC_parse_path(TYPES_YML.c_str()), ".%s", name.c_str());
}

/// The datatype of a data that %PDI has been initialized with
PDI::Datatype_sptr datatype(const std::string& data_name)
{
	auto&& store = PDI::Data_store::store();
	return store.desc(data_name).default_type()->evaluate(store);
}

struct DocDatatype: public ::PDI::PdiTest {};

/// Every example is a valid specification tree whose data PDI accepts, their datatypes evaluated with the metadata they use set to zero.
TEST_F(DocDatatype, examples)
{
	alignas(std::max_align_t) static char buffer[1 << 16] = {};
	PC_tree_t examples = PC_parse_path(TYPES_YML.c_str());
	int nb_examples = 0;
	ASSERT_EQ(PC_OK, PC_len(examples, &nb_examples));
	for (int example_id = 0; example_id < nb_examples; ++example_id) {
		char* name = nullptr;
		ASSERT_EQ(PC_OK, PC_string(PC_get(examples, "{%d}", example_id), &name));
		std::string const example_name = name;
		free(name);
#ifndef PDI_DOC_FORTRAN
		if (example_name.starts_with("spectree_fortran")) continue;
#endif
		SCOPED_TRACE(example_name);
		PC_tree_t tree = example(example_name);
		InitPdi(tree);
		for (auto&& section: {".metadata", ".data"}) {
			PC_errhandler_t pc_handler = PC_errhandler(PC_NULL_HANDLER);
			PC_tree_t names = PC_get(tree, "%s", section);
			PC_errhandler(pc_handler);
			if (PC_status(names)) continue;
			int nb_names = 0;
			ASSERT_EQ(PC_OK, PC_len(names, &nb_names));
			for (int name_id = 0; name_id < nb_names; ++name_id) {
				char* data_name = nullptr;
				ASSERT_EQ(PC_OK, PC_string(PC_get(names, "{%d}", name_id), &data_name));
				ASSERT_LE(datatype(data_name)->buffersize(), sizeof(buffer)) << data_name;
				EXPECT_EQ(PDI_OK, PDI_expose(data_name, buffer, PDI_OUT)) << data_name;
				free(data_name);
			}
		}
		FinalizePdi();
	}
	PC_tree_destroy(&examples);
}

/// The scalar form of a datatype is a shortcut for a mapping with a single type key.
TEST_F(DocDatatype, scalar_shortcut)
{
	InitPdi(PC_parse_string("data: {iter: int, expanded: {type: int}}"));
	EXPECT_EQ(*datatype("expanded"), *datatype("iter"));
}

/// The attribute of the example is given to the datatype.
TEST_F(DocDatatype, attribute)
{
	InitPdi(example("spectree_datatype_attribute_name"));
	PDI::Datatype_template_sptr field_type = PDI::Data_store::store().desc("field").default_type();
	ASSERT_EQ(1, field_type->attributes().size());
	EXPECT_EQ(1, field_type->attributes().count("decl_hdf5.chunking"));
}

/// The struct example has the layout of the C struct that the reference says it matches, and its members are those of an instance of it.
TEST_F(DocDatatype, struct_layout)
{
	InitPdi(example("spectree_struct"));
	PDI::Datatype_sptr type = datatype("particle");

	particle data{3.5, 42};
	ASSERT_EQ(sizeof(data), type->buffersize());
	EXPECT_EQ(offsetof(particle, position), static_cast<char*>(type->member("position", &data).first) - reinterpret_cast<char*>(&data));
	EXPECT_EQ(offsetof(particle, id), static_cast<char*>(type->member("id", &data).first) - reinterpret_cast<char*>(&data));

	PDI::Ref_r ref{&data, [](void*) {}, type, true, false};
	EXPECT_EQ(3.5, *static_cast<const double*>(PDI::Ref_r{ref["position"]}.get()));
	EXPECT_EQ(42, *static_cast<const int64_t*>(PDI::Ref_r{ref["id"]}.get()));
}

} // namespace
