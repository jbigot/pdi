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
/// The functions that the examples of the User-code configuration reference (reference.yml) call, built into the program that runs them.
/// Each accesses the aliases that the examples give it, with the rights they share them with, so that their names and rights are checked.

#include <pdi.h>

/// Accesses an alias with some rights, then releases it
static void use(const char* alias, PDI_inout_t access)
{
	void* buffer = NULL;
	if (!PDI_access(alias, &buffer, access)) {
		PDI_release(alias);
	}
}

void write_checkpoint(void) {}

void report(void) {}

void fun2(void) {}

void process_field(void)
{
	use("input", PDI_IN);
}

/// Called with in and out by some examples, with in only by another: out is only accessed when it is there
void compute(void)
{
	use("in", PDI_IN);
	PDI_errhandler_t handler = PDI_errhandler(PDI_NULL_HANDLER);
	void* buffer = NULL;
	if (!PDI_access("out", &buffer, PDI_OUT)) {
		PDI_release("out");
	}
	PDI_errhandler(handler);
}

void fun1(void)
{
	use("in", PDI_IN);
	use("out", PDI_OUT);
}
