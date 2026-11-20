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

//! [example]
#include <stdio.h>

#include <pdi.h>

int main(int argc, char* argv[])
{
	PDI_init(PC_parse_path("posix.yml"));

	int rank = 0;
	PDI_expose("rank", &rank, PDI_OUT);

	// start from scratch
	remove("field_0.bin");
	int checkpoint_found;
	PDI_expose("checkpoint_found", &checkpoint_found, PDI_IN);
	if (checkpoint_found) return 1;

	// compute, the plugin writes the field on iterations 0 and 10
	int restart = 0;
	PDI_expose("restart", &restart, PDI_OUT);
	double field[64][64];
	for (int iter = 0; iter <= 10; ++iter) {
		for (int i = 0; i < 64; ++i)
			for (int j = 0; j < 64; ++j)
				field[i][j] = iter * 10000 + i * 64 + j;
		PDI_expose("iter", &iter, PDI_OUT);
		PDI_expose("field", field, PDI_INOUT);
	}

	// restart, the plugin reads the field back as it was on iteration 10
	PDI_expose("checkpoint_found", &checkpoint_found, PDI_IN);
	if (!checkpoint_found) return 1;
	for (int i = 0; i < 64; ++i)
		for (int j = 0; j < 64; ++j)
			field[i][j] = 0;
	restart = 1;
	PDI_expose("restart", &restart, PDI_OUT);
	int iter = 11;
	PDI_expose("iter", &iter, PDI_OUT);
	PDI_expose("field", field, PDI_INOUT);
	for (int i = 0; i < 64; ++i)
		for (int j = 0; j < 64; ++j)
			if (field[i][j] != 10 * 10000 + i * 64 + j) return 1;

	PDI_finalize();
	return 0;
}

//! [example]
