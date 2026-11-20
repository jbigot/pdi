#=============================================================================
# Copyright (C) 2026 Commissariat a l'energie atomique et aux energies alternatives (CEA)
#
# All rights reserved.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions are met:
# * Redistributions of source code must retain the above copyright notice,
#   this list of conditions and the following disclaimer.
# * Redistributions in binary form must reproduce the above copyright notice,
#   this list of conditions and the following disclaimer in the documentation
#   and/or other materials provided with the distribution.
# * Neither the names of CEA, nor the names of the contributors may be used to
#   endorse or promote products derived from this software without specific
#   prior written  permission.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
# AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
# IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
# ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
# LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
# CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
# SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
# INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
# CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
# ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
# POSSIBILITY OF SUCH DAMAGE.
#=============================================================================

cmake_minimum_required(VERSION 3.22...4.2)

# The code of the plugin creation tutorial (How_to_create_a_plugin.md): building it ensures the tutorial keeps matching the plugin API, and
# running the programs that load the plugins ensures they work.

add_library(doc_minimal_plugin MODULE minimal_plugin.cxx)
target_link_libraries(doc_minimal_plugin PDI::PDI_plugins)

# The plugins that are loaded are named as PDI expects to find them.
add_library(doc_example_plugin MODULE example_plugin.cxx)
target_link_libraries(doc_example_plugin PDI::PDI_plugins)
set_target_properties(doc_example_plugin PROPERTIES OUTPUT_NAME pdi_example_plugin PREFIX lib)

add_library(doc_posix_plugin MODULE posix.cxx)
target_link_libraries(doc_posix_plugin PDI::PDI_plugins)
set_target_properties(doc_posix_plugin PROPERTIES OUTPUT_NAME pdi_posix_plugin PREFIX lib)

add_executable(doc_example_use example_use.c)
target_link_libraries(doc_example_use PDI::PDI_C)

add_executable(doc_posix_use posix_use.c)
target_link_libraries(doc_posix_use PDI::PDI_C)

if("${BUILD_TESTING}")
	# the programs read their specification tree from the current directory, and posix_use writes files there
	configure_file(example_use.yml example_use.yml COPYONLY)
	configure_file(posix.yml posix.yml COPYONLY)
	add_test(NAME How_to_create_a_plugin_example_use COMMAND doc_example_use WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}")
	add_test(NAME How_to_create_a_plugin_posix_use COMMAND doc_posix_use WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}")
	set_tests_properties(How_to_create_a_plugin_example_use How_to_create_a_plugin_posix_use PROPERTIES
			ENVIRONMENT "PDI_PLUGIN_PATH=$<TARGET_FILE_DIR:doc_example_plugin>")
endif()
