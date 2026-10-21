/*******************************************************************************
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


#include <algorithm>
#include <array>
#include <numeric>
#include <vector>

#include <pdi/testing.h>

#include "serialized_records.h"

using PDI::combination_tests::expect_eq_grid;
using PDI::combination_tests::expect_eq_subvector;
using PDI::combination_tests::expect_eq_vector;
using PDI::combination_tests::Grid;
using PDI::combination_tests::init_record;
using PDI::combination_tests::Owned_record;
using PDI::combination_tests::Subvector;
using PDI::combination_tests::Vector;
using testing::ElementsAreArray;

/** \file
 *
 * Integration tests of the Serialize plugin writing and reading records through the Decl'HDF5 plugin
 */

struct SerializeDeclHdf5: public ::PDI::PdiTest {};

namespace {

/// A record with a pointer member, as exposed by the application
struct Record_data {
	int a;
	int* b;
};

} // namespace

// a scalar, an array section and a record with a pointer, written then read back
TEST_F(SerializeDeclHdf5, ScalarArrayAndRecord)
{
	InitPdi(PC_parse_string(R"==(
logging: trace
metadata:
  input: int
data:
  scalar_data: int
  array_data:
    type: array
    subtype: int
    size: 8
    subsize: 4
    start: 2
  record_data:
    type: record
    buffersize: 16
    members:
      a:
        disp: 0
        type: int
      b:
        disp: 8
        type: pointer
        subtype: int
plugins:
  serialize:
    scalar_data: scalar_data_serialized
    array_data: array_data_serialized
    record_data: record_data_serialized
  decl_hdf5:
    - file: ScalarArrayAndRecord.h5
      when: $input=0
      write: [scalar_data_serialized, array_data_serialized, record_data_serialized]
    - file: ScalarArrayAndRecord.h5
      when: $input=1
      read: [scalar_data_serialized, array_data_serialized, record_data_serialized]
)=="));

	int input = 0;
	int scalar_data = 42;
	std::array<int, 8> array_data;
	std::iota(array_data.begin(), array_data.end(), 42);
	int b = 51;
	Record_data record_data{50, &b};
	ASSERT_EQ(PDI_OK, PDI_expose("input", &input, PDI_OUT));
	ASSERT_EQ(PDI_OK, PDI_expose("scalar_data", &scalar_data, PDI_OUT));
	ASSERT_EQ(PDI_OK, PDI_expose("array_data", array_data.data(), PDI_OUT));
	ASSERT_EQ(PDI_OK, PDI_expose("record_data", &record_data, PDI_OUT));

	input = 1;
	ASSERT_EQ(PDI_OK, PDI_expose("input", &input, PDI_OUT));

	int scalar_data_read = 0;
	ASSERT_EQ(PDI_OK, PDI_expose("scalar_data", &scalar_data_read, PDI_IN));
	EXPECT_EQ(scalar_data, scalar_data_read);

	// only the section [2, 6) of the array is serialized
	std::array<int, 8> array_data_read{};
	ASSERT_EQ(PDI_OK, PDI_expose("array_data", array_data_read.data(), PDI_IN));
	EXPECT_THAT(
		std::vector(array_data_read.begin() + 2, array_data_read.begin() + 6),
		ElementsAreArray(array_data.begin() + 2, array_data.begin() + 6)
	);

	int b_read = 0;
	Record_data record_data_read{0, &b_read};
	ASSERT_EQ(PDI_OK, PDI_expose("record_data", &record_data_read, PDI_IN));
	EXPECT_EQ(record_data.a, record_data_read.a);
	EXPECT_EQ(b, b_read);
}

// a record with pointers to an array and to a record, written then read back
TEST_F(SerializeDeclHdf5, Subvector)
{
	InitPdi(PC_parse_string(R"==(
.types:
  - &Subregion
    type: record
    buffersize: 56
    members:
        ix:      {disp: 0, type: int}
        iy:      {disp: 4, type: int}
        iz:      {disp: 8, type: int}
        nx:      {disp: 12, type: int}
        ny:      {disp: 16, type: int}
        nz:      {disp: 20, type: int}
        sx:      {disp: 24, type: int}
        sy:      {disp: 28, type: int}
        sz:      {disp: 32, type: int}
        rx:      {disp: 36, type: int}
        ry:      {disp: 40, type: int}
        rz:      {disp: 44, type: int}
        level:   {disp: 48, type: int}
        process: {disp: 52, type: int}
  - &Subvector
    type: record
    buffersize: 32
    members:
      data:
        disp: 0
        type: pointer
        subtype:
            type: array
            size: 16
            subtype: double
      allocated:
        disp: 8
        type: int
      data_space:
        disp: 16
        type: pointer
        subtype: *Subregion
      data_size:
        disp: 24
        type: int
logging: trace
data:
  subvector: *Subvector
plugins:
  serialize:
    subvector: subvector_serialized
  decl_hdf5:
    - file: Subvector.h5
      on_event: write
      write: [subvector_serialized]
    - file: Subvector.h5
      on_event: read
      read: [subvector_serialized]
)=="));

	Owned_record<Subvector> expected;
	init_record(*expected);

	{
		Owned_record<Subvector> data_write;
		init_record(*data_write);
		ASSERT_EQ(PDI_OK, PDI_multi_expose("write", "subvector", &*data_write, PDI_OUT, nullptr));
	}

	Owned_record<Subvector> data_read;
	ASSERT_EQ(PDI_OK, PDI_share("subvector", &*data_read, PDI_IN));
	ASSERT_EQ(PDI_OK, PDI_event("read"));
	ASSERT_EQ(PDI_OK, PDI_reclaim("subvector"));

	expect_eq_subvector(*data_read, *expected);
}

// a record with nested arrays of arrays of pointers to records, written then read back
TEST_F(SerializeDeclHdf5, Grid)
{
	InitPdi(PC_parse_string(R"==(
.types:
  - &Subregion
    type: record
    buffersize: 56
    members:
        ix:      {disp: 0, type: int}
        iy:      {disp: 4, type: int}
        iz:      {disp: 8, type: int}
        nx:      {disp: 12, type: int}
        ny:      {disp: 16, type: int}
        nz:      {disp: 20, type: int}
        sx:      {disp: 24, type: int}
        sy:      {disp: 28, type: int}
        sz:      {disp: 32, type: int}
        rx:      {disp: 36, type: int}
        ry:      {disp: 40, type: int}
        rz:      {disp: 44, type: int}
        level:   {disp: 48, type: int}
        process: {disp: 52, type: int}
  - &SubregionArray
    type: record
    buffersize: 16
    members:
      subregions:
        disp: 0
        type: pointer
        subtype:
          type: array
          size: 64
          subtype:
            type: pointer
            subtype: *Subregion
      size:
        disp: 8
        type: int
  - &Grid
    type: record
    buffersize: 32
    members:
      subgrids:
        disp: 0
        type: pointer
        subtype:
            type: array
            size: 32
            subtype: *SubregionArray
      all_subgrids:
          disp: 8
          type: pointer
          subtype:
            type: array
            size: 32
            subtype: *SubregionArray
      neighbors:
          disp: 16
          type: pointer
          subtype:
            type: array
            size: 32
            subtype: *SubregionArray
      size:
        disp: 24
        type: int

logging: trace
metadata:
  input: int
data:
  grid_data: *Grid
plugins:
  serialize:
    grid_data: grid_data_serialized
  decl_hdf5:
    - file: Grid.h5
      when: $input=0
      write: [grid_data_serialized]
    - file: Grid.h5
      when: $input=1
      read: [grid_data_serialized]
)=="));

	Owned_record<Grid> expected;
	init_record(*expected);

	{
		Owned_record<Grid> data_write;
		init_record(*data_write);
		int input = 0;
		ASSERT_EQ(PDI_OK, PDI_expose("input", &input, PDI_OUT));
		ASSERT_EQ(PDI_OK, PDI_expose("grid_data", &*data_write, PDI_OUT));
	}

	Owned_record<Grid> data_read;
	int input = 1;
	ASSERT_EQ(PDI_OK, PDI_expose("input", &input, PDI_OUT));
	ASSERT_EQ(PDI_OK, PDI_expose("grid_data", &*data_read, PDI_IN));

	expect_eq_grid(*data_read, *expected);
}

// a vector record and an array section, written then read back
TEST_F(SerializeDeclHdf5, VectorAndArray)
{
	InitPdi(PC_parse_string(R"==(
types:
  subvector:
    type: record
    buffersize: 32
    members:
        data:
          disp: 0
          type: pointer
          subtype:
              type: array
              size: 16
              subtype: double
        allocated:
          disp: 8
          type: int
        data_space:
          disp: 16
          type: pointer
          subtype: subregion
        data_size:
          disp: 24
          type: int
  subregionArray:
    type: record 
    buffersize: 16
    members:
      subregions:
        disp: 0
        type: pointer
        subtype:
          type: array
          size: 64
          subtype:
            type: pointer
            subtype: subregion
      size:
        disp: 8
        type: int
  subregion:
    type: record
    buffersize: 56
    members:
        ix:      {disp: 0, type: int}
        iy:      {disp: 4, type: int}
        iz:      {disp: 8, type: int}
        nx:      {disp: 12, type: int}
        ny:      {disp: 16, type: int}
        nz:      {disp: 20, type: int}
        sx:      {disp: 24, type: int}
        sy:      {disp: 28, type: int}
        sz:      {disp: 32, type: int}
        rx:      {disp: 36, type: int}
        ry:      {disp: 40, type: int}
        rz:      {disp: 44, type: int}
        level:   {disp: 48, type: int}
        process: {disp: 52, type: int}
  grid:
    type: record
    buffersize: 32
    members:
      subgrids:
        disp: 0
        type: pointer
        subtype:
            type: array
            size: 32
            subtype: subregionArray
      all_subgrids:
          disp: 8
          type: pointer
          subtype:
            type: array
            size: 32
            subtype: subregionArray
      neighbors:
          disp: 16
          type: pointer
          subtype:
            type: array
            size: 32
            subtype: subregionArray
      size:
        disp: 24
        type: int
  vector:
    type: record
    buffersize: 40
    members:
      subvectors:
        disp: 0
        type: pointer
        subtype:
          type: array
          size: 8
          subtype:
            type: pointer
            subtype: subvector
      data_size:
        disp: 8
        type: int
      grid:
        disp: 16
        type: pointer
        subtype: grid
      data_space:
        disp: 24
        type: pointer
        subtype: subregionArray
      size:
        disp: 32
        type: int
logging: trace
data:
  int_array:
    type: array
    subtype: int
    size: [10, 10]
    subsize: [8, 8]
    start: [1, 1]
  vector_data: vector
plugins:
  serialize:
    vector_data: vector_data_serialized
    int_array: int_array_serialized
  decl_hdf5:
    - file: VectorAndArray.h5
      on_event: write
      write: [vector_data_serialized, int_array_serialized]
    - file: VectorAndArray.h5
      on_event: read
      read: [vector_data_serialized, int_array_serialized]
)=="));

	Owned_record<Vector> expected;
	init_record(*expected);

	// the elements of the section hold their value, the border is not serialized
	std::array<std::array<int, 10>, 10> array_write;
	std::array<std::array<int, 10>, 10> array_expected{};
	for (int i = 0; i < 10; i++) {
		for (int j = 0; j < 10; j++) {
			array_write[i][j] = 10 * i + j;
			if (i >= 1 && i < 9 && j >= 1 && j < 9) {
				array_expected[i][j] = 10 * i + j;
			}
		}
	}

	{
		Owned_record<Vector> data_write;
		init_record(*data_write);
		ASSERT_EQ(PDI_OK, PDI_multi_expose("write", "int_array", array_write.data(), PDI_OUT, "vector_data", &*data_write, PDI_OUT, nullptr));
	}

	Owned_record<Vector> data_read;
	std::array<std::array<int, 10>, 10> array_read{};
	ASSERT_EQ(PDI_OK, PDI_multi_expose("read", "int_array", array_read.data(), PDI_IN, "vector_data", &*data_read, PDI_IN, nullptr));

	EXPECT_EQ(array_read, array_expected);
	expect_eq_vector(*data_read, *expected);
}
