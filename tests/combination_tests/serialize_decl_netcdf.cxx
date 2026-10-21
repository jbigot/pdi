/*******************************************************************************
 * Copyright (C) 2021 Institute of Bioorganic Chemistry Polish Academy of Science (PSNC)
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

#include <array>

#include <pdi/testing.h>

#include "serialized_records.h"

using PDI::combination_tests::expect_eq_vector;
using PDI::combination_tests::init_record;
using PDI::combination_tests::Owned_record;
using PDI::combination_tests::Vector;

/** \file
 *
 * Integration test of the Serialize plugin writing and reading records through the Decl'NetCDF plugin
 */

struct SerializeDeclNetcdf: public ::PDI::PdiTest {};

// a vector record and an array section, written then read back (the large serialized record needs the _NoFill attribute)
TEST_F(SerializeDeclNetcdf, VectorAndArray)
{
	InitPdi(PC_parse_string(R"==(
types:
  subvector:
    type: struct
    +decl_netcdf.type: subvector
    members:
      - data:
          type: pointer
          subtype:
              type: array
              size: 16
              subtype: double
      - allocated: int
      - data_space:
          type: pointer
          subtype: subregion
      - data_size:
          type: int
  subregionArray:
    type: struct
    +decl_netcdf.type: subregionArray
    members:
      - subregions:
          type: pointer
          subtype:
            type: array
            size: 64
            subtype:
              type: pointer
              subtype: subregion
      - size: int
  subregion:
    type: struct
    +decl_netcdf.type: subregion
    members:
      - ix: int
      - iy: int
      - iz: int
      - nx: int
      - ny: int
      - nz: int
      - sx: int
      - sy: int
      - sz: int
      - rx: int
      - ry: int
      - rz: int
      - level: int
      - process: int
  grid:
    type: struct
    +decl_netcdf.type: grid
    members:
      - subgrids:
          type: pointer
          subtype:
              type: array
              size: 32
              subtype: subregionArray
      - all_subgrids:
          type: pointer
          subtype:
            type: array
            size: 32
            subtype: subregionArray
      - neighbors:
          type: pointer
          subtype:
            type: array
            size: 32
            subtype: subregionArray
      - size: int
  vector:
    type: struct
    +decl_netcdf.type: vector
    members:
      - subvectors:
          type: pointer
          subtype:
            type: array
            size: 8
            subtype:
              type: pointer
              subtype: subvector
      - data_size: int
      - grid:
          type: pointer
          subtype: grid
      - data_space:
          type: pointer
          subtype: subregionArray
      - size: int
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
  decl_netcdf:
    - file: VectorAndArray.nc
      on_event: write
      variables:
        vector_data_serialized:
          attributes:
            _NoFill: true
      write: [vector_data_serialized, int_array_serialized]
    - file: VectorAndArray.nc
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
