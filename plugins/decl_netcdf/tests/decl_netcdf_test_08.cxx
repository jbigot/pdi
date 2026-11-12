/*******************************************************************************
 * Copyright (C) 2026 Commissariat a l'energie atomique et aux energies alternatives (CEA)
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
#include <cstdint>
#include <filesystem>
#include <iostream>
#include <numeric>
#include <ranges>
#include <utility>
#include <vector>

#include <netcdf.h>

#include <pdi/testing.h>


using PDI::make_random;
using PDI::random_init;

struct Record_t {
	int int_scalar;
	std::array<double, 32> double_array;

	bool operator== (const Record_t&) const = default;

	void init_from(std::uniform_random_bit_generator auto& gen)
	{
		random_init(gen, int_scalar);
		random_init(gen, double_array);
	}
};

std::ostream& operator<< (std::ostream& out, Record_t const & r)
{
	out << "Record_t(int_scalar=" << r.int_scalar << ", double_array=(";

	const char* sep = "";
	for (auto&& value: r.double_array) {
		out << sep << value;
		sep = ", ";
	}
	out << "))";

	return out;
}

class DeclNetcdf: public ::PDI::PdiTest
{};

// Tests simple write and read of scalar and array depending on event
TEST_F(DeclNetcdf, RecordScalarAndArray)
{
	InitPdi(PC_parse_string(R"==(
logging: trace
data:
  record:
    type: struct
    +decl_netcdf.type: record_type
    members:
      - int_scalar: int
      - double_array:
          type: array
          subtype: double
          size: 32
plugins:
  decl_netcdf:
    - file: "test_08.nc"
      on_event: "write"
      write:
        record:
          variable: /data/record
    - file: "test_08.nc"
      on_event: "read"
      read:
        record:
          variable: /data/record
)=="));

	auto const write_record_data = make_a<Record_t>();

	// write data
	PDI_multi_expose("write", "record", &write_record_data, PDI_OUT, NULL);

	Record_t read_record_data;

	// read data
	PDI_multi_expose("read", "record", &read_record_data, PDI_IN, NULL);

	EXPECT_EQ(read_record_data, write_record_data);
}

/// A record whose size is above the 64 KiB limit of an HDF5 object header message
struct Large_record_t {
	int int_scalar;
	std::array<double, 9000> double_array;

	void init_from(std::uniform_random_bit_generator auto& gen)
	{
		random_init(gen, int_scalar);
		random_init(gen, double_array);
	}
};

static_assert(sizeof(Large_record_t) > 65536, "the record must not fit in an HDF5 object header message");

// Tests write and read of a record larger than 64 KiB, whose fill value HDF5 can not store, with the _NoFill attribute
TEST_F(DeclNetcdf, LargeRecord)
{
	InitPdi(PC_parse_string(R"==(
logging: trace
data:
  record:
    type: struct
    +decl_netcdf.type: large_record_type
    members:
      - int_scalar: int
      - double_array:
          type: array
          subtype: double
          size: 9000
plugins:
  decl_netcdf:
    - file: "test_08_large.nc"
      on_event: "write"
      variables:
        /data/record:
          attributes:
            _NoFill: true
      write:
        record:
          variable: /data/record
    - file: "test_08_large.nc"
      on_event: "read"
      read:
        record:
          variable: /data/record
)=="));

	auto const write_record_data = make_a<Large_record_t>();

	// write data
	ASSERT_EQ(PDI_OK, PDI_multi_expose("write", "record", &write_record_data, PDI_OUT, NULL));

	// value-initialized so that the read can not pass unless it stores the data
	Large_record_t read_record_data{};

	// read data
	ASSERT_EQ(PDI_OK, PDI_multi_expose("read", "record", &read_record_data, PDI_IN, NULL));

	EXPECT_EQ(read_record_data.int_scalar, write_record_data.int_scalar);
	EXPECT_THAT(read_record_data.double_array, ::testing::ElementsAreArray(write_record_data.double_array));
}

/// The default fill value of the NetCDF `int` type, NC_FILL_INT in netcdf.h
constexpr int NC_FILL_INT_VALUE = -2147483647;

// Tests that the elements of a variable that are not written read back as the default fill value
TEST_F(DeclNetcdf, PartialWriteDefaultFillValue)
{
	InitPdi(PC_parse_string(R"==(
logging: trace
data:
  part: {type: array, subtype: int, size: 5}
  whole: {type: array, subtype: int, size: 10}
plugins:
  decl_netcdf:
    - file: "test_08_fill.nc"
      on_event: "write"
      variables:
        v: {type: array, subtype: int, size: 10}
      write:
        part:
          variable: v
          variable_selection: {start: [0], subsize: [5]}
    - file: "test_08_fill.nc"
      on_event: "read"
      variables:
        v: {type: array, subtype: int, size: 10}
      read:
        whole:
          variable: v
)=="));

	std::array<int, 5> const write_data{1, 2, 3, 4, 5};

	// write only the first half of the variable
	ASSERT_EQ(PDI_OK, PDI_multi_expose("write", "part", write_data.data(), PDI_OUT, NULL));

	// not a fill value so that the read can not pass unless it stores the data
	std::array<int, 10> read_data;
	read_data.fill(-1);

	// read the whole variable
	ASSERT_EQ(PDI_OK, PDI_multi_expose("read", "whole", read_data.data(), PDI_IN, NULL));

	EXPECT_THAT(
		read_data,
		::testing::ElementsAre(1, 2, 3, 4, 5, NC_FILL_INT_VALUE, NC_FILL_INT_VALUE, NC_FILL_INT_VALUE, NC_FILL_INT_VALUE, NC_FILL_INT_VALUE)
	);
}

// Tests that the elements of a variable that are not written read back as its _FillValue attribute when it has one
TEST_F(DeclNetcdf, PartialWriteFillValueAttribute)
{
	InitPdi(PC_parse_string(R"==(
logging: trace
metadata:
  fv: int
data:
  part: {type: array, subtype: int, size: 5}
  whole: {type: array, subtype: int, size: 10}
plugins:
  decl_netcdf:
    - file: "test_08_fill_value.nc"
      on_event: "write"
      variables:
        v: {type: array, subtype: int, size: 10, attributes: {_FillValue: $fv}}
      write:
        part:
          variable: v
          variable_selection: {start: [0], subsize: [5]}
    - file: "test_08_fill_value.nc"
      on_event: "read"
      variables:
        v: {type: array, subtype: int, size: 10}
      read:
        whole:
          variable: v
)=="));

	int const fill_value = 42;
	std::array<int, 5> const write_data{1, 2, 3, 4, 5};

	// write only the first half of the variable
	ASSERT_EQ(PDI_OK, PDI_multi_expose("write", "fv", &fill_value, PDI_OUT, "part", write_data.data(), PDI_OUT, NULL));

	// not the fill value so that the read can not pass unless it stores the data
	std::array<int, 10> read_data;
	read_data.fill(-1);

	// read the whole variable
	ASSERT_EQ(PDI_OK, PDI_multi_expose("read", "whole", read_data.data(), PDI_IN, NULL));

	EXPECT_THAT(read_data, ::testing::ElementsAre(1, 2, 3, 4, 5, fill_value, fill_value, fill_value, fill_value, fill_value));
}

// Tests that the _NoFill attribute disables the fill value of a variable without being written as an attribute
TEST_F(DeclNetcdf, NoFillAttribute)
{
	InitPdi(PC_parse_string(R"==(
logging: trace
data:
  w_true: {type: array, subtype: int, size: 4}
  w_int: {type: array, subtype: int, size: 4}
  w_false: {type: array, subtype: int, size: 4}
  w_default: {type: array, subtype: int, size: 4}
plugins:
  decl_netcdf:
    - file: "test_08_no_fill.nc"
      on_event: "write"
      variables:
        v_true: {type: array, subtype: int, size: 4, attributes: {_NoFill: "true"}}
        v_int: {type: array, subtype: int, size: 4, attributes: {_NoFill: 1}}
        v_false: {type: array, subtype: int, size: 4, attributes: {_NoFill: "false"}}
        v_default: {type: array, subtype: int, size: 4}
      write:
        w_true: {variable: v_true}
        w_int: {variable: v_int}
        w_false: {variable: v_false}
        w_default: {variable: v_default}
)=="));

	std::array<int, 4> const write_data{1, 2, 3, 4};
	ASSERT_EQ(
		PDI_OK,
		PDI_multi_expose(
			"write",
			"w_true",
			write_data.data(),
			PDI_OUT,
			"w_int",
			write_data.data(),
			PDI_OUT,
			"w_false",
			write_data.data(),
			PDI_OUT,
			"w_default",
			write_data.data(),
			PDI_OUT,
			NULL
		)
	);
	// close the file
	FinalizePdi();

	int file_id;
	ASSERT_EQ(NC_NOERR, nc_open("test_08_no_fill.nc", NC_NOWRITE, &file_id));
	for (auto&& [name, expected_no_fill]: std::vector<std::pair<const char*, int>>{{"v_true", 1}, {"v_int", 1}, {"v_false", 0}, {"v_default", 0}}) {
		SCOPED_TRACE(name);
		int var_id;
		ASSERT_EQ(NC_NOERR, nc_inq_varid(file_id, name, &var_id));
		// not written as an ordinary attribute
		EXPECT_EQ(NC_ENOTATT, nc_inq_att(file_id, var_id, "_NoFill", nullptr, nullptr));
		int no_fill = -1;
		EXPECT_EQ(NC_NOERR, nc_inq_var_fill(file_id, var_id, &no_fill, nullptr));
		EXPECT_EQ(expected_no_fill, no_fill);
	}
	EXPECT_EQ(NC_NOERR, nc_close(file_id));
}

// Tests that the _NoFill attribute reads back as the fill mode of the variable
TEST_F(DeclNetcdf, NoFillAttributeRead)
{
	InitPdi(PC_parse_string(R"==(
logging: trace
data:
  w_true: {type: array, subtype: int, size: 4}
  w_false: {type: array, subtype: int, size: 4}
  values: {type: array, subtype: int, size: 4}
  no_fill: int
  no_fill_byte: uint8
plugins:
  decl_netcdf:
    - file: "test_08_no_fill_read.nc"
      on_event: "write"
      variables:
        v_true: {type: array, subtype: int, size: 4, attributes: {_NoFill: true}}
        v_false: {type: array, subtype: int, size: 4}
      write:
        w_true: {variable: v_true}
        w_false: {variable: v_false}
    - file: "test_08_no_fill_read.nc"
      on_event: "read_true"
      variables:
        v_true: {type: array, subtype: int, size: 4, attributes: {_NoFill: $no_fill}}
      read:
        values: {variable: v_true}
    - file: "test_08_no_fill_read.nc"
      on_event: "read_false"
      variables:
        v_false: {type: array, subtype: int, size: 4, attributes: {_NoFill: $no_fill_byte}}
      read:
        values: {variable: v_false}
)=="));

	std::array<int, 4> const write_data{1, 2, 3, 4};
	ASSERT_EQ(PDI_OK, PDI_multi_expose("write", "w_true", write_data.data(), PDI_OUT, "w_false", write_data.data(), PDI_OUT, NULL));

	std::array<int, 4> read_data{};
	int no_fill = 42;
	ASSERT_EQ(PDI_OK, PDI_multi_expose("read_true", "values", read_data.data(), PDI_IN, "no_fill", &no_fill, PDI_IN, NULL));
	EXPECT_EQ(1, no_fill);

	uint8_t no_fill_byte = 42;
	ASSERT_EQ(PDI_OK, PDI_multi_expose("read_false", "values", read_data.data(), PDI_IN, "no_fill_byte", &no_fill_byte, PDI_IN, NULL));
	EXPECT_EQ(0, no_fill_byte);
}
