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
/// The plugin the first steps of the plugin creation tutorial (How_to_create_a_plugin.md) build up.
/// Each step is a snippet of this one plugin, which example_use loads.

//! [includes]
#include <string>
#include <unordered_map>
#include <vector>

#include <pdi/context.h>
#include <pdi/error.h>
#include <pdi/logger.h>
#include <pdi/paraconf_wrapper.h>
#include <pdi/plugin.h>
#include <pdi/ref_any.h>

//! [includes]

struct example_plugin: public PDI::Plugin {
	example_plugin(PDI::Logger& logger, PDI::Context& ctx, PC_tree_t spec_tree)
		: Plugin{logger, ctx}
	{
		//! [on_data]
		ctx.on_data([this](const std::string& data_name, PDI::Ref ref) { this->logger().info("User has shared a data named {}", data_name); });
		//! [on_data]

		//! [rw_data]
		ctx.on_data([this](const std::string& data_name, PDI::Ref ref) {
			// this plugin reads the data: ask for read access
			if (PDI::Ref_r ref_r{ref}) {
				// converts the value to long, whatever the type of integer, and throws if it is not a scalar
				long some_integer = ref_r.scalar_value<long>();
				this->logger().info("{} = {}", data_name, some_integer);
			} else {
				throw PDI::Permission_error{"Cannot read `{}'", data_name};
			}
		});
		//! [rw_data]

		//! [events]
		ctx.on_event([this](const std::string& event_name) { this->handle_event(event_name); });
		ctx.on_event(
			[this](const std::string& event_name) { this->handle_configured_event(event_name); },
			PDI::to_string(PC_get(spec_tree, ".on_event"))
		);
		//! [events]

		//! [conf_scalar_array]
		// scalar
		PC_tree_t scalar_tree = PC_get(spec_tree, ".scalar");
		std::string string_scalar = PDI::to_string(scalar_tree);

		// array
		PC_tree_t array_tree = PC_get(spec_tree, ".array");
		int array_size = PDI::len(array_tree);
		std::vector<long> array;
		for (int i = 0; i < array_size; i++) {
			PC_tree_t array_element = PC_get(array_tree, "[%d]", i);
			array.emplace_back(PDI::to_long(array_element));
		}
		//! [conf_scalar_array]

		//! [conf_map]
		PC_tree_t subtree = PC_get(spec_tree, ".custom_subtree");
		int subtree_size = PDI::len(subtree);
		std::unordered_map<std::string, long> custom_map;
		for (int i = 0; i < subtree_size; i++) {
			PC_tree_t key = PC_get(subtree, "{%d}", i);
			PC_tree_t value = PC_get(subtree, "<%d>", i);
			custom_map.emplace(PDI::to_string(key), PDI::to_long(value));
		}
		//! [conf_map]
	}

private:
	//! [event_handlers]
	void handle_event(const std::string& event_name) { logger().info("Event `{}' called", event_name); }

	void handle_configured_event(const std::string& event_name) { logger().info("Configured event `{}' called", event_name); }

	//! [event_handlers]
};

PDI_PLUGIN(example)
