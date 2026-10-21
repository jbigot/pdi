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


#include <ostream>
#include <string>
#include <vector>

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "serialized_records.h"

namespace PDI::combination_tests {

namespace {

// Fills the subregion with the same value everywhere
void init_subregion(Subregion& subregion, int value)
{
	subregion = {value, value, value, value, value, value, value, value, value, value, value, value, value, value};
}

void init_subregion_array(SubregionArray& subregion_array, int value)
{
	for (int i = 0; i < SubregionArray_size; i++) {
		init_subregion(*subregion_array.subregions[i], i + value);
	}
	subregion_array.size = SubregionArray_size;
}

void alloc_subregion_array(SubregionArray& subregion_array)
{
	subregion_array.subregions = new Subregion*[SubregionArray_size];
	for (int i = 0; i < SubregionArray_size; i++) {
		subregion_array.subregions[i] = new Subregion();
	}
}

void free_subregion_array(SubregionArray& subregion_array)
{
	for (int i = 0; i < SubregionArray_size; i++) {
		delete subregion_array.subregions[i];
	}
	delete[] subregion_array.subregions;
}

SubregionArray* alloc_subregion_arrays()
{
	auto* arrays = new SubregionArray[Grid_size];
	for (int i = 0; i < Grid_size; i++) {
		alloc_subregion_array(arrays[i]);
	}
	return arrays;
}

void free_subregion_arrays(SubregionArray* arrays)
{
	for (int i = 0; i < Grid_size; i++) {
		free_subregion_array(arrays[i]);
	}
	delete[] arrays;
}

void init_subregion_arrays(SubregionArray* arrays, int value)
{
	for (int i = 0; i < Grid_size; i++) {
		init_subregion_array(arrays[i], value);
	}
}

void expect_eq_subregion_array(const SubregionArray& actual, const SubregionArray& expected)
{
	for (int i = 0; i < SubregionArray_size; i++) {
		SCOPED_TRACE("subregion " + std::to_string(i));
		EXPECT_EQ(*actual.subregions[i], *expected.subregions[i]);
	}
	EXPECT_EQ(actual.size, expected.size);
}

void expect_eq_subregion_arrays(const SubregionArray* actual, const SubregionArray* expected)
{
	for (int i = 0; i < Grid_size; i++) {
		SCOPED_TRACE("subregion array " + std::to_string(i));
		expect_eq_subregion_array(actual[i], expected[i]);
	}
}

} // namespace

bool Subregion::operator== (const Subregion&) const = default;

std::ostream& operator<< (std::ostream& out, const Subregion& s)
{
	return out << "Subregion(i=(" << s.ix << ", " << s.iy << ", " << s.iz << "), n=(" << s.nx << ", " << s.ny << ", " << s.nz << "), s=(" << s.sx
	           << ", " << s.sy << ", " << s.sz << "), r=(" << s.rx << ", " << s.ry << ", " << s.rz << "), level=" << s.level
	           << ", process=" << s.process << ")";
}

void alloc_record(Subvector& subvector)
{
	subvector.data = new double[Subvector_data_size]();
	subvector.data_space = new Subregion();
}

void free_record(Subvector& subvector)
{
	delete[] subvector.data;
	delete subvector.data_space;
}

void init_record(Subvector& subvector)
{
	for (int i = 0; i < Subvector_data_size; i++) {
		subvector.data[i] = static_cast<double>(i);
	}
	subvector.allocated = 1;
	init_subregion(*subvector.data_space, 42);
	subvector.data_size = Subvector_data_size;
}

void alloc_record(Grid& grid)
{
	grid.subgrids = alloc_subregion_arrays();
	grid.all_subgrids = alloc_subregion_arrays();
	grid.neighbors = alloc_subregion_arrays();
}

void free_record(Grid& grid)
{
	free_subregion_arrays(grid.subgrids);
	free_subregion_arrays(grid.all_subgrids);
	free_subregion_arrays(grid.neighbors);
}

void init_record(Grid& grid)
{
	init_subregion_arrays(grid.subgrids, 10);
	init_subregion_arrays(grid.all_subgrids, 100);
	init_subregion_arrays(grid.neighbors, 200);
	grid.size = Grid_size;
}

void alloc_record(Vector& vector)
{
	vector.subvectors = new Subvector*[Vector_data_size];
	for (int i = 0; i < Vector_data_size; i++) {
		vector.subvectors[i] = new Subvector();
		alloc_record(*vector.subvectors[i]);
	}
	vector.grid = new Grid();
	alloc_record(*vector.grid);
	vector.data_space = new SubregionArray();
	alloc_subregion_array(*vector.data_space);
}

void free_record(Vector& vector)
{
	for (int i = 0; i < Vector_data_size; i++) {
		free_record(*vector.subvectors[i]);
		delete vector.subvectors[i];
	}
	delete[] vector.subvectors;
	free_record(*vector.grid);
	delete vector.grid;
	free_subregion_array(*vector.data_space);
	delete vector.data_space;
}

void init_record(Vector& vector)
{
	for (int i = 0; i < Vector_data_size; i++) {
		init_record(*vector.subvectors[i]);
	}
	vector.data_size = Vector_data_size;
	init_record(*vector.grid);
	init_subregion_array(*vector.data_space, 0);
	vector.size = Vector_size;
}

void expect_eq_subvector(const Subvector& actual, const Subvector& expected)
{
	EXPECT_THAT(std::vector<double>(actual.data, actual.data + Subvector_data_size), testing::ElementsAreArray(expected.data, Subvector_data_size));
	EXPECT_EQ(actual.allocated, expected.allocated);
	EXPECT_EQ(*actual.data_space, *expected.data_space);
	EXPECT_EQ(actual.data_size, expected.data_size);
}

void expect_eq_grid(const Grid& actual, const Grid& expected)
{
	{
		SCOPED_TRACE("subgrids");
		expect_eq_subregion_arrays(actual.subgrids, expected.subgrids);
	}
	{
		SCOPED_TRACE("all_subgrids");
		expect_eq_subregion_arrays(actual.all_subgrids, expected.all_subgrids);
	}
	{
		SCOPED_TRACE("neighbors");
		expect_eq_subregion_arrays(actual.neighbors, expected.neighbors);
	}
	EXPECT_EQ(actual.size, expected.size);
}

void expect_eq_vector(const Vector& actual, const Vector& expected)
{
	EXPECT_EQ(actual.data_size, expected.data_size);
	EXPECT_EQ(actual.size, expected.size);
	for (int i = 0; i < Vector_data_size; i++) {
		SCOPED_TRACE("subvector " + std::to_string(i));
		expect_eq_subvector(*actual.subvectors[i], *expected.subvectors[i]);
	}
	{
		SCOPED_TRACE("grid");
		expect_eq_grid(*actual.grid, *expected.grid);
	}
	{
		SCOPED_TRACE("data_space");
		expect_eq_subregion_array(*actual.data_space, *expected.data_space);
	}
}

} // namespace PDI::combination_tests
