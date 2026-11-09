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
 * * Neither the name of CEA nor the names of its contributors may be used to
 *   endorse or promote products derived from this software without specific
 *   prior written permission.
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
/// The program of the example of the Pycall plugin documentation (README.md), run with the pycall_ex tree of reference.yml.
/// The documentation shows its code, and its output, which the test compares to pycall_example.output.
/// usage: pycall_example <reference.yml>

#include <stdio.h>

#include <paraconf.h>
#include <pdi.h>

int main(int argc, char* argv[])
{
	if (argc != 2) {
		fprintf(stderr, "usage: %s <reference.yml>\n", argv[0]);
		return 1;
	}
	// so that the lines of C, of Python and of the logs reach the output in the order they are written, as on a terminal
	setvbuf(stdout, NULL, _IONBF, 0);
	PDI_init(PC_get(PC_parse_path(argv[1]), ".pycall_ex"));

	//! [pycall_ex]
	int a[3] = {1, 2, 3};

	printf(" * [C] starting with $a = [%d %d %d]\n", a[0], a[1], a[2]);

	PDI_share("a", a, PDI_INOUT);
	PDI_event("testing");
	PDI_reclaim("a");

	printf(" * [C] now I see     $a = [%d %d %d]\n", a[0], a[1], a[2]);
	//! [pycall_ex]

	PDI_finalize();
	return 0;
}
