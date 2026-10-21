#=============================================================================
# Copyright (C) 2020 Institute of Bioorganic Chemistry Polish Academy of Science (PSNC)
# Copyright (C) 2020-2026 Commissariat a l'energie atomique et aux energies alternatives (CEA)
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

cmake_minimum_required(VERSION 3.22...4.2)
project(pdi_tests LANGUAGES C CXX)

find_package(Threads REQUIRED)
if(NOT TARGET GTest::gtest)
	option(INSTALL_GTEST "Enable installation of googletest. (Projects embedding googletest may want to turn this OFF.)" OFF)
	add_subdirectory("../../vendor/googletest-56efe39/" "googletest" EXCLUDE_FROM_ALL)
endif()
include(GoogleTest)

option(BUILD_DECL_HDF5_PLUGIN "Build Decl'HDF5 plug-in" ON)
option(BUILD_SERIALIZE_PLUGIN "Build Serialize plug-in" ON)

include(CTest)

find_package(PDI REQUIRED COMPONENTS C)

add_executable(integration_tests include.cxx)
target_link_libraries(integration_tests PDI::PDI_C GTest::gmock GTest::gmock_main GTest::gtest GTest::gtest_main)
gtest_discover_tests(integration_tests)

if("${BUILD_DECL_HDF5_PLUGIN}" AND "${BUILD_SERIALIZE_PLUGIN}")

add_executable(serialize_decl_hdf5_tests serialize_decl_hdf5.cxx serialized_records.cxx)
target_link_libraries(serialize_decl_hdf5_tests PDI::PDI_C GTest::gmock_main)
gtest_discover_tests(serialize_decl_hdf5_tests)

endif("${BUILD_DECL_HDF5_PLUGIN}" AND "${BUILD_SERIALIZE_PLUGIN}")
if("${BUILD_DECL_NETCDF_PLUGIN}" AND "${BUILD_SERIALIZE_PLUGIN}")

add_executable(serialize_decl_netcdf_tests serialize_decl_netcdf.cxx serialized_records.cxx)
target_link_libraries(serialize_decl_netcdf_tests PDI::PDI_C GTest::gmock_main)
gtest_discover_tests(serialize_decl_netcdf_tests)

endif("${BUILD_DECL_NETCDF_PLUGIN}" AND "${BUILD_SERIALIZE_PLUGIN}")
