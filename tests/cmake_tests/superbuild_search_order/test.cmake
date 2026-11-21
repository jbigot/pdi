#=============================================================================
# Copyright (C) 2026 Julien Bigot <julien@julien-bigot.fr>
# All rights reserved.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions are met:
# * Redistributions of source code must retain the above copyright
#   notice, this list of conditions and the following disclaimer.
# * Redistributions in binary form must reproduce the above copyright
#   notice, this list of conditions and the following disclaimer in the
#   documentation and/or other materials provided with the distribution.
# * Neither the name of CEA nor the names of its contributors may be used to
#   endorse or promote products derived from this software without specific
#   prior written permission.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
# THE SOFTWARE.
#=============================================================================

# Checks which copy of libyaml the superbuild makes the bundled paraconf find.
# With USE_yaml=EMBEDDED it must be the copy built into the staging tree, whatever CMAKE_PREFIX_PATH and yaml_ROOT say and whichever copy an earlier
# configuration of the same build tree used.
# With USE_yaml=SYSTEM it must be the copy the user points at, although the staging tree still holds the one built before.
# usage: cmake -DPDI_DIST_DIR=<root of the PDI distribution> -DWORK_DIR=<scratch directory>
#              [-DGENERATOR=<generator>] [-DC_COMPILER=<compiler>] [-DCXX_COMPILER=<compiler>] -P test.cmake

cmake_minimum_required(VERSION 3.22...4.2)

set(DECOY "${WORK_DIR}/decoy")
set(BUILD "${WORK_DIR}/build")
set(TOOLCHAIN_FLAGS)
if(NOT "${GENERATOR}" STREQUAL "")
	list(APPEND TOOLCHAIN_FLAGS -G "${GENERATOR}")
endif()
foreach(LANG C CXX)
	if(NOT "${${LANG}_COMPILER}" STREQUAL "")
		list(APPEND TOOLCHAIN_FLAGS "-DCMAKE_${LANG}_COMPILER=${${LANG}_COMPILER}")
	endif()
endforeach()

function(run)
	execute_process(COMMAND ${ARGN} RESULT_VARIABLE RESULT OUTPUT_VARIABLE OUTPUT ERROR_VARIABLE OUTPUT)
	if(NOT "${RESULT}" EQUAL 0)
		string(REPLACE ";" " " COMMAND_TEXT "${ARGN}")
		message(FATAL_ERROR "`${COMMAND_TEXT}' failed:\n${OUTPUT}")
	endif()
endfunction()

# Configures the distribution with the flags given after the three named parameters, builds the bundled paraconf,
# then checks in its cache where it found libyaml (yaml_DIR), which yaml_ROOT it was handed, and that staging is the first prefix it was handed.
# An expected value is a directory the cache value must be in; an empty one means the cache value must be empty.
function(check PHASE EXPECTED_DIR EXPECTED_ROOT)
	run("${CMAKE_COMMAND}" -S "${PDI_DIST_DIR}" -B "${BUILD}" ${TOOLCHAIN_FLAGS} ${ARGN})
	run("${CMAKE_COMMAND}" --build "${BUILD}" --target paraconf_pkg)
	foreach(NAME DIR ROOT)
		file(STRINGS "${BUILD}/paraconf/src/paraconf_pkg-build/CMakeCache.txt" VALUE REGEX "^yaml_${NAME}:[A-Z]+=")
		string(REGEX REPLACE "^[^=]*=" "" VALUE "${VALUE}")
		if(NOT "${VALUE}" STREQUAL "")
			file(REAL_PATH "${VALUE}" VALUE)
		endif()
		set(EXPECTED "${EXPECTED_${NAME}}")
		if("${EXPECTED}" STREQUAL "")
			string(COMPARE EQUAL "${VALUE}" "" AS_EXPECTED)
		else()
			file(REAL_PATH "${EXPECTED}" EXPECTED)
			string(FIND "${VALUE}/" "${EXPECTED}/" POSITION)
			string(COMPARE EQUAL "${POSITION}" "0" AS_EXPECTED)
		endif()
		if(NOT "${AS_EXPECTED}")
			message(FATAL_ERROR "${PHASE}: the bundled paraconf has yaml_${NAME}=`${VALUE}', expected `${EXPECTED}'")
		endif()
	endforeach()
	# The staging tree must be the first prefix handed, for what is looked up without a <name>_ROOT of its own
	file(STRINGS "${BUILD}/paraconf/src/paraconf_pkg-build/CMakeCache.txt" VALUE REGEX "^CMAKE_PREFIX_PATH:[A-Z]+=")
	string(REGEX REPLACE "^[^=]*=([^;\\]*).*$" "\\1" VALUE "${VALUE}")
	file(REAL_PATH "${VALUE}" VALUE)
	file(REAL_PATH "${BUILD}/staging" EXPECTED)
	if(NOT "${VALUE}" STREQUAL "${EXPECTED}")
		message(FATAL_ERROR "${PHASE}: the first prefix handed to the bundled paraconf is `${VALUE}', expected `${EXPECTED}'")
	endif()
	message(STATUS "${PHASE}: passed")
endfunction()

file(REMOVE_RECURSE "${WORK_DIR}")

# What an inherited environment names would be searched before the prefixes (the upper-case spelling too, since CMake 3.27): only the test sets it
foreach(PACKAGE yaml paraconf)
	string(TOUPPER "${PACKAGE}" PACKAGE_UPPER)
	unset(ENV{${PACKAGE}_ROOT})
	unset(ENV{${PACKAGE_UPPER}_ROOT})
endforeach()

# The decoy: the bundled libyaml, installed where the user's settings will point
file(GLOB YAML_SOURCE "${PDI_DIST_DIR}/vendor/libyaml-*")
run("${CMAKE_COMMAND}" -S "${YAML_SOURCE}" -B "${WORK_DIR}/decoy-build" ${TOOLCHAIN_FLAGS} "-DCMAKE_INSTALL_PREFIX=${DECOY}"
	-DBUILD_SHARED_LIBS=ON -DBUILD_TESTING=OFF -DCMAKE_POLICY_VERSION_MINIMUM=3.10 -DINSTALL_CMAKE_DIR=share/yaml/cmake)
run("${CMAKE_COMMAND}" --build "${WORK_DIR}/decoy-build")
run("${CMAKE_COMMAND}" --install "${WORK_DIR}/decoy-build")

check("1: bundled, another copy in CMAKE_PREFIX_PATH" "${BUILD}/staging" "${BUILD}/staging"
	-DUSE_DEFAULT=EMBEDDED -DBUILD_DECL_HDF5_PLUGIN=OFF -DBUILD_DECL_NETCDF_PLUGIN=OFF -DBUILD_DOCUMENTATION=OFF -DBUILD_FORTRAN=OFF
	-DBUILD_MPI_PLUGIN=OFF -DBUILD_PYCALL_PLUGIN=OFF -DBUILD_PYTHON=OFF -DBUILD_TESTING=OFF "-DCMAKE_PREFIX_PATH=${DECOY}")
check("2: switched to the system copy, the one of CMAKE_PREFIX_PATH" "${DECOY}" ""
	-DUSE_yaml=SYSTEM)
set(ENV{yaml_ROOT} "${DECOY}")
check("3: switched back to bundled, another copy in yaml_ROOT, variable and environment" "${BUILD}/staging" "${BUILD}/staging"
	-DUSE_yaml=EMBEDDED "-Dyaml_ROOT=${DECOY}")
# Nothing but the environment names the decoy now: the first pass must find it there, and hand an empty yaml_ROOT in place of the staging one
check("4: switched to the system copy, the one of the environment's yaml_ROOT" "${DECOY}" ""
	-DUSE_yaml=SYSTEM -Uyaml_ROOT -Uyaml_DIR -DCMAKE_PREFIX_PATH=)
check("5: the system copy, the one of the yaml_ROOT variable" "${DECOY}" "${DECOY}"
	"-Dyaml_ROOT=${DECOY}")
