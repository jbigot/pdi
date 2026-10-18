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


# Configure-only: check how pdi_deprecated_alias turns a deprecated cache name into the default of the name that replaces it.

cmake_minimum_required(VERSION 3.22...4.2)
project(pdi_cmake_tests_deprecated_alias LANGUAGES NONE)

include("${CMAKE_CURRENT_SOURCE_DIR}/../assert.cmake")
include("${CMAKE_CURRENT_SOURCE_DIR}/../../../cmake/DeprecatedAlias.cmake")

# Each case uses names of its own, so that they all run in one configuration.
function(check_alias NAME EXPECTED_NEW)
	pdi_deprecated_alias("${NAME}_OLD" "${NAME}_NEW" "the new name")
	string(COMPARE EQUAL "$CACHE{${NAME}_NEW}" "${EXPECTED_NEW}" _NEW_AS_EXPECTED)
	assert("${_NEW_AS_EXPECTED}")
	# The old name must be gone, or a superbuild would forward it to its dependencies.
	if(DEFINED "CACHE{${NAME}_OLD}")
		set(_OLD_REMOVED FALSE)
	else()
		set(_OLD_REMOVED TRUE)
	endif()
	assert("${_OLD_REMOVED}")
endfunction()

# Neither name given: the new one is left for its own declaration.
check_alias(NEITHER "")

# The old name only: it becomes the value of the new one.
set(OLD_ONLY_OLD "old" CACHE PATH "")
check_alias(OLD_ONLY "old")

# Both, agreeing.
set(AGREE_OLD "same" CACHE PATH "")
set(AGREE_NEW "same" CACHE PATH "")
check_alias(AGREE "same")

# Both, disagreeing: the new name wins.
set(DISAGREE_OLD "old" CACHE PATH "")
set(DISAGREE_NEW "new" CACHE PATH "")
check_alias(DISAGREE "new")
