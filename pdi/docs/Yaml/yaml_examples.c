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
/// The C examples of \ref YAML, run from the directory of example.yaml and format.yaml: the trees of example.yaml are fed to %PDI, so that
/// they are checked to be valid specification trees and not merely valid YAML; the plain YAML of format.yaml is only parsed, as YAML is all it
/// illustrates.

#include <paraconf.h>
#include <pdi.h>

int main(int argc, char* argv[])
{
	//! [init_whole_file]
	PDI_init(PC_parse_path("example.yaml"));
	//! [init_whole_file]
	PDI_finalize();

	//! [init_subtree]
	PDI_init(PC_get(PC_parse_path("example.yaml"), ".pdi_subtree"));
	//! [init_subtree]
	PDI_finalize();

	// the plain YAML only has to be valid YAML, paraconf is enough
	PC_tree_t format = PC_parse_path("format.yaml");
	PC_tree_destroy(&format);

	return 0;
}
