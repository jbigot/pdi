/*******************************************************************************
 * Copyright (C) 2025-2026 Commissariat a l'energie atomique et aux energies alternatives (CEA)
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 * * Redistributions of source code must retain the above copyright
 *   notice, this list of conditions and the following disclaimer.
 * * Redistributions in binary form must reproduce the above copyright
 *   notice, this list of conditions and the following disclaimer in the
 *   documentation and/or other materials provided with the distribution.
 * * Neither the name of CEA nor the names of its contributors may be used to
 *   endorse or promote products derived from this software without specific
 *   prior written permission.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 ******************************************************************************/

#include <iostream>
#include <type_traits>

#include <pdi.h>

#include "test_api.h"

using Pdi_version_t = std::remove_cv_t<decltype(PDI_VERSION)>;
extern "C" PDI_status_t PDI_version_signature(Pdi_version_t*, Pdi_version_t);
static_assert(
	std::is_same_v<decltype(PDI_version), decltype(PDI_version_signature)>,
	"PDI_version() parameters should be the same as that of PDI_VERSION"
);

// The version macros must be usable in a preprocessor #if
#if PDI_VERSION < PDI_COMPUTE_VERSION(1, 0, 0) || PDI_VERSION >= PDI_COMPUTE_VERSION(PDI_VERSION_MAJOR + 1, 0, 0)
#error PDI_VERSION and PDI_COMPUTE_VERSION disagree in the preprocessor
PDI_VERSION and PDI_COMPUTE_VERSION disagree in the preprocessor !;
#endif

using Pdi_version_t = std::remove_cv_t<decltype(PDI_VERSION)>;
static_assert(std::is_same_v<Pdi_version_t, unsigned long>, "PDI_VERSION is not an unsigned long");
static_assert(std::is_same_v<decltype(PDI_COMPUTE_VERSION(1, 2, 3)), Pdi_version_t>, "PDI_COMPUTE_VERSION and PDI_VERSION type differ");
static_assert(PDI_COMPUTE_VERSION(1, 2, 3) == 0x01020300, "PDI_COMPUTE_VERSION has the wrong layout");
static_assert(PDI_COMPUTE_VERSION(255, 255, 255) == 0xFFFFFF00, "PDI_COMPUTE_VERSION overflows its components");
static_assert(PDI_VERSION == PDI_COMPUTE_VERSION(PDI_VERSION_MAJOR, PDI_VERSION_MINOR, PDI_VERSION_PATCH), "PDI_VERSION is not computed correctly");

int main(int argc, char* argv[])
{
	tests(argc, argv);
}
