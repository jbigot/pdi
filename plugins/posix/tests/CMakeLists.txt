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

option(BUILD_FORTRAN "Enable Fortran support" ON)

set(PDI_COMPONENTS C)
if("${BUILD_FORTRAN}")
	enable_language(Fortran)
	list(APPEND PDI_COMPONENTS f90)
endif()
find_package(PDI REQUIRED COMPONENTS ${PDI_COMPONENTS})

set(LANGUAGES C)
add_executable(posix_use_C posix_use.c)
target_link_libraries(posix_use_C PDI::PDI_C)
if("${BUILD_FORTRAN}")
	list(APPEND LANGUAGES F)
	add_executable(posix_use_F posix_use.f90)
	target_link_libraries(posix_use_F PDI::PDI_f90)
endif()

# Each program checkpoints, then restarts from its checkpoint, in a directory of its own; its output, which the tutorial shows, is compared to it.
foreach(LANGUAGE IN LISTS LANGUAGES)
	file(MAKE_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/${LANGUAGE}")
	add_test(NAME posix_plugin_${LANGUAGE}
			COMMAND PDI::pdirun "${CMAKE_COMMAND}"
				"-DEXAMPLE=$<TARGET_FILE:posix_use_${LANGUAGE}>"
				"-DARGS=${CMAKE_CURRENT_SOURCE_DIR}/posix.yml"
				"-DEXPECTED=${CMAKE_CURRENT_SOURCE_DIR}/posix_use.output"
				"-DPLUGIN_PATH=$<TARGET_FILE_DIR:pdi_posix_plugin>"
				"-DWORKDIR=${CMAKE_CURRENT_BINARY_DIR}/${LANGUAGE}"
				-P "${CMAKE_CURRENT_SOURCE_DIR}/../../../pdi/docs/run_and_compare.cmake")
endforeach()
