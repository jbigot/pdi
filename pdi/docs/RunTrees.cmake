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

# pdi_doc_run_trees(<name> <trees.yml> [MPI] [NO_SHARE] [INOUT] [VALUES <metadata>=<int>...] [EVENTS <event>...] [INIT_ONLY <tree>...]
#                   [SOURCES <file>...] [PLUGIN_PATH <dir>])
#
# Adds a test <name>.<tree> for each specification tree of <trees.yml>, a file of documentation examples whose root keys name the trees.
# Each runs run_trees.cxx: PDI is initialized with the tree, each of its metadata is exposed and each of its data shared, the EVENTS are emitted,
# and the data reclaimed.
# The metadata are zero, but those given a value in VALUES, exposed as an int.
# With NO_SHARE, nothing is shared, for the trees whose plugins share the data themselves; otherwise the data is shared for reading only,
# or for reading and writing with INOUT.
# The SOURCES are built into the program, whose symbols are exported, for the functions that the trees name.
# PLUGIN_PATH is searched for plugins first, for plugins built beside the test.
# MPI is initialized around the run with MPI; the trees listed after INIT_ONLY are only initialized.
# The tests run in a directory of their own, <name>.run, in the current binary directory, where the trees write their files; each fails if a
# plugin logs an error.
function(pdi_doc_run_trees NAME TREES)
	cmake_parse_arguments(PARSE_ARGV 2 _RUN_TREES "MPI;NO_SHARE;INOUT" "PLUGIN_PATH" "VALUES;EVENTS;INIT_ONLY;SOURCES")
	set(_RUN_TREES_ARGS)
	if("${_RUN_TREES_NO_SHARE}")
		list(APPEND _RUN_TREES_ARGS --no-share)
	endif()
	if("${_RUN_TREES_INOUT}")
		list(APPEND _RUN_TREES_ARGS --inout)
	endif()
	foreach(_RUN_TREES_VALUE IN LISTS _RUN_TREES_VALUES)
		list(APPEND _RUN_TREES_ARGS --value "${_RUN_TREES_VALUE}")
	endforeach()
	foreach(_RUN_TREES_EVENT IN LISTS _RUN_TREES_EVENTS)
		list(APPEND _RUN_TREES_ARGS --event "${_RUN_TREES_EVENT}")
	endforeach()

	add_executable(${NAME} "${CMAKE_CURRENT_FUNCTION_LIST_DIR}/run_trees.cxx" ${_RUN_TREES_SOURCES})
	if(NOT "${_RUN_TREES_SOURCES}" STREQUAL "")
		set_target_properties(${NAME} PROPERTIES ENABLE_EXPORTS TRUE)
	endif()
	target_link_libraries(${NAME} PRIVATE PDI::PDI_C)
	target_compile_features(${NAME} PRIVATE cxx_std_17)
	if("${_RUN_TREES_MPI}")
		find_package(MPI REQUIRED COMPONENTS C)
		target_link_libraries(${NAME} PRIVATE MPI::MPI_C)
		target_compile_definitions(${NAME} PRIVATE RUN_TREES_MPI)
	endif()
	file(MAKE_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/${NAME}.run")

	# the names of the trees, the keys at the root of the file
	set_property(DIRECTORY APPEND PROPERTY CMAKE_CONFIGURE_DEPENDS "${TREES}")
	file(STRINGS "${TREES}" _RUN_TREES_NAMES REGEX "^[A-Za-z0-9_]+:")
	list(TRANSFORM _RUN_TREES_NAMES REPLACE ":.*" "")
	foreach(_RUN_TREES_NAME IN LISTS _RUN_TREES_INIT_ONLY)
		if(NOT "${_RUN_TREES_NAME}" IN_LIST _RUN_TREES_NAMES)
			message(FATAL_ERROR "pdi_doc_run_trees: no tree ${_RUN_TREES_NAME} in ${TREES}")
		endif()
	endforeach()
	foreach(_RUN_TREES_NAME IN LISTS _RUN_TREES_NAMES)
		set(_RUN_TREES_TREE_ARGS ${_RUN_TREES_ARGS})
		if("${_RUN_TREES_NAME}" IN_LIST _RUN_TREES_INIT_ONLY)
			list(APPEND _RUN_TREES_TREE_ARGS --init-only)
		endif()
		add_test(NAME ${NAME}.${_RUN_TREES_NAME}
				COMMAND PDI::pdirun "$<TARGET_FILE:${NAME}>" "${TREES}" "${_RUN_TREES_NAME}" ${_RUN_TREES_TREE_ARGS}
				WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/${NAME}.run")
		# a plugin can log an error and go on, as Pycall does for an exception of the Python code
		set_property(TEST ${NAME}.${_RUN_TREES_NAME} PROPERTY FAIL_REGULAR_EXPRESSION "\\*\\*\\* error:")
		if(NOT "${_RUN_TREES_PLUGIN_PATH}" STREQUAL "")
			set_property(TEST ${NAME}.${_RUN_TREES_NAME} PROPERTY ENVIRONMENT_MODIFICATION "PDI_PLUGIN_PATH=path_list_prepend:${_RUN_TREES_PLUGIN_PATH}")
		endif()
	endforeach()
endfunction()
