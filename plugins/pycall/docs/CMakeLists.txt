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

include("${CMAKE_CURRENT_SOURCE_DIR}/../../../pdi/docs/RunTrees.cmake")

# The specification trees of the documentation (reference.yml), run by PDI.
pdi_doc_run_trees(pycall_reference "${CMAKE_CURRENT_SOURCE_DIR}/reference.yml" INOUT EVENTS testing my_event)

# The program of the example, whose output the documentation shows, compared to it.
add_executable(pycall_example pycall_example.c)
target_link_libraries(pycall_example PDI::PDI_C)
add_test(NAME pycall_example
		COMMAND PDI::pdirun "${CMAKE_COMMAND}"
			"-DEXAMPLE=$<TARGET_FILE:pycall_example>"
			"-DARGS=${CMAKE_CURRENT_SOURCE_DIR}/reference.yml"
			"-DEXPECTED=${CMAKE_CURRENT_SOURCE_DIR}/pycall_example.output"
			-P "${CMAKE_CURRENT_SOURCE_DIR}/../../../pdi/docs/run_and_compare.cmake")
set_property(TEST pycall_example PROPERTY ENVIRONMENT PYTHONUNBUFFERED=1)
