/*******************************************************************************
 * Copyright (C) 2020 Commissariat a l'energie atomique et aux energies alternatives (CEA)
 * Copyright (C) 2020 Institute of Bioorganic Chemistry Polish Academy of Science (PSNC)
 * Copyright (C) 2026 Julien Bigot <julien@julien-bigot.fr>
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


#ifndef TESTS_COMBINATION_TESTS_SERIALIZED_RECORDS_H_
#define TESTS_COMBINATION_TESTS_SERIALIZED_RECORDS_H_

#include <iosfwd>

/** \file
 *
 * The ParFlow-like records shared by the combination tests, with the helpers to build them and to compare them through GoogleTest.
 *
 * The layout of the structures is described by the YAML of the tests, so it must not change.
 */

namespace PDI::combination_tests {

// Structures from ParFlow (DO NOT CHANGE -> The same values are in the YAML of the tests)
inline constexpr int SubregionArray_size = 64;
inline constexpr int Grid_size = 32;
inline constexpr int Subvector_data_size = 16;
inline constexpr int Vector_data_size = 8;
inline constexpr int Vector_size = 4;

struct Subregion {
	int ix, iy, iz; ///< Bottom-lower-left corner in index-space
	int nx, ny, nz; ///< Size
	int sx, sy, sz; ///< Striding factors
	int rx, ry, rz; ///< Refinement over the background grid
	int level; ///< Refinement level = rx + ry + rz
	int process; ///< Process containing this subgrid

	bool operator== (const Subregion&) const;
};

struct SubregionArray {
	Subregion** subregions; ///< Array of pointers to subregions
	int size; ///< Size of subgregion array
};

struct Grid {
	SubregionArray* subgrids; ///< Array of subgrids in this process
	SubregionArray* all_subgrids; ///< Array of all subgrids in the grid
	SubregionArray* neighbors; ///< Array of nearest neighbor subgrids
	int size; ///< Total number of grid points
};

struct Subvector {
	double* data; ///< Pointer to subvector data
	int allocated; ///< Was this data allocated?
	Subregion* data_space; ///< Pointer to data space
	int data_size; ///< Number of elements in vector, includes ghost points
};

struct Vector {
	Subvector** subvectors; ///< Array of pointers to subvectors
	int data_size; ///< Number of elements in vector. All subvectors. includes ghost points
	Grid* grid; ///< Grid that this vector is on
	SubregionArray* data_space; ///< Description of Vector data
	int size; ///< Total number of coefficients
};

std::ostream& operator<< (std::ostream& out, const Subregion& subregion);

/// Allocates the memory of a record, the record content is left to init_record
void alloc_record(Subvector& subvector);
void alloc_record(Grid& grid);
void alloc_record(Vector& vector);

/// Releases the memory allocated by alloc_record
void free_record(Subvector& subvector);
void free_record(Grid& grid);
void free_record(Vector& vector);

/// Fills an allocated record with the reference values
void init_record(Subvector& subvector);
void init_record(Grid& grid);
void init_record(Vector& vector);

/** A record that owns its memory, freed whatever way the test exits.
 *
 * The record is allocated, but not initialized: use init_record or give it the content to read into.
 */
template <typename Record>
class Owned_record
{
	Record m_record{};

public:
	Owned_record() { alloc_record(m_record); }

	Owned_record(const Owned_record&) = delete;

	Owned_record& operator= (const Owned_record&) = delete;

	~Owned_record() { free_record(m_record); }

	Record& operator* () { return m_record; }

	const Record& operator* () const { return m_record; }

	Record* operator->() { return &m_record; }

	const Record* operator->() const { return &m_record; }
};

/** Checks, with EXPECT_*, that two records hold the same values.
 *
 * Any difference fails the running test, with a trace naming the element.
 * Unlike with ASSERT_*, the test goes on after a failure.
 */
void expect_eq_subvector(const Subvector& actual, const Subvector& expected);
void expect_eq_grid(const Grid& actual, const Grid& expected);
void expect_eq_vector(const Vector& actual, const Vector& expected);

} // namespace PDI::combination_tests

#endif // TESTS_COMBINATION_TESTS_SERIALIZED_RECORDS_H_
