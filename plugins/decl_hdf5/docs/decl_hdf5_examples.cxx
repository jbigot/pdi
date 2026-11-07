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
/// Runs the specification trees of the Decl'HDF5 examples page
/// (plugins/decl_hdf5/examples.md) with the program it describes, and checks
/// the files they write against what the page says of them.

#include <cstdint>
#include <filesystem>
#include <string>
#include <vector>

#include <stdlib.h>
#include <unistd.h>

#include <gtest/gtest.h>
#include <hdf5.h>
#ifdef H5_HAVE_PARALLEL
#include <mpi.h>
#endif

#include <pdi.h>

namespace {

/// The path of the specification tree of an example
std::string tree(const std::string& name)
{
	return std::string(EXAMPLES_DIR) + "/" + name + ".yml";
}

/// The rank and number of processes, 0 and 1 without MPI
void world(int& rank, int& nprocs)
{
	rank = 0;
	nprocs = 1;
#ifdef H5_HAVE_PARALLEL
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	MPI_Comm_size(MPI_COMM_WORLD, &nprocs);
#endif
}

/// The value the program gives to a cell of the interior of field, by its global row
double value(int iter, int row, int column)
{
	return 100 * iter + 10 * row + column;
}

/// The program that the page describes: it shares rank, nprocs and size, then iter and field at each of 3 iterations.
/// It returns the first error, or PDI_OK.
PDI_status_t run(const std::string& name)
{
	PDI_status_t status = PDI_init(PC_parse_path(tree(name).c_str()));
	if (status) return status;
	int rank, nprocs;
	world(rank, nprocs);
	int size[2] = {4, 6};
	status = PDI_multi_expose("init", "rank", &rank, PDI_OUT, "nprocs", &nprocs, PDI_OUT, "size", size, PDI_OUT, NULL);
	double field[4 + 2][6 + 2];
	for (int iter = 0; iter < 3 && !status; ++iter) {
		for (int i = 0; i < 4 + 2; ++i) {
			for (int j = 0; j < 6 + 2; ++j) {
				bool ghost = (i == 0 || i == 4 + 1 || j == 0 || j == 6 + 1);
				field[i][j] = ghost ? -1 : value(iter, rank * 4 + i - 1, j - 1);
			}
		}
		status = PDI_multi_expose("iteration", "iter", &iter, PDI_OUT, "field", field, PDI_OUT, NULL);
	}
	PDI_status_t finalize_status = PDI_finalize();
	return status ? status : finalize_status;
}

/// The 6x8 block of field at an iteration, ghost cells included, of rank 0
std::vector<double> block(int iter)
{
	std::vector<double> result;
	for (int i = 0; i < 4 + 2; ++i) {
		for (int j = 0; j < 6 + 2; ++j) {
			bool ghost = (i == 0 || i == 4 + 1 || j == 0 || j == 6 + 1);
			result.push_back(ghost ? -1 : value(iter, i - 1, j - 1));
		}
	}
	return result;
}

/// The interior of field at an iteration, for the given number of processes, one after the other
std::vector<double> interior(int iter, int nprocs = 1)
{
	std::vector<double> result;
	for (int row = 0; row < nprocs * 4; ++row) {
		for (int column = 0; column < 6; ++column) {
			result.push_back(value(iter, row, column));
		}
	}
	return result;
}

/// A dataset of a file: its dimensions and its values, read as double
struct Dataset {
	std::vector<hsize_t> dims;
	std::vector<double> values;
	size_t type_size;
};

bool exists(const std::string& file, const std::string& path)
{
	hid_t f = H5Fopen(file.c_str(), H5F_ACC_RDONLY, H5P_DEFAULT);
	if (f < 0) return false;
	bool result = H5Lexists(f, path.c_str(), H5P_DEFAULT) > 0;
	H5Fclose(f);
	return result;
}

Dataset read(const std::string& file, const std::string& path)
{
	Dataset result;
	hid_t f = H5Fopen(file.c_str(), H5F_ACC_RDONLY, H5P_DEFAULT);
	EXPECT_GE(f, 0) << file;
	hid_t d = H5Dopen2(f, path.c_str(), H5P_DEFAULT);
	EXPECT_GE(d, 0) << file << ":" << path;
	hid_t space = H5Dget_space(d);
	result.dims.resize(H5Sget_simple_extent_ndims(space));
	H5Sget_simple_extent_dims(space, result.dims.data(), NULL);
	result.values.resize(H5Sget_simple_extent_npoints(space));
	H5Dread(d, H5T_NATIVE_DOUBLE, H5S_ALL, H5S_ALL, H5P_DEFAULT, result.values.data());
	hid_t type = H5Dget_type(d);
	result.type_size = H5Tget_size(type);
	H5Tclose(type);
	H5Sclose(space);
	H5Dclose(d);
	H5Fclose(f);
	return result;
}

int int_attribute(const std::string& file, const std::string& path, const std::string& name)
{
	int result = -1;
	hid_t f = H5Fopen(file.c_str(), H5F_ACC_RDONLY, H5P_DEFAULT);
	hid_t a = H5Aopen_by_name(f, path.c_str(), name.c_str(), H5P_DEFAULT, H5P_DEFAULT);
	EXPECT_GE(a, 0) << file << ":" << path << "#" << name;
	H5Aread(a, H5T_NATIVE_INT, &result);
	H5Aclose(a);
	H5Fclose(f);
	return result;
}

/// Each example runs in a directory of its own, removed afterwards
class DeclHdf5Examples: public ::testing::Test
{
	std::filesystem::path m_previous;
	std::filesystem::path m_dir;

protected:
	void SetUp() override
	{
		m_previous = std::filesystem::current_path();
		int rank, nprocs;
		world(rank, nprocs);
		std::string dir = "decl_hdf5_examples-XXXXXX";
		if (rank == 0) {
			ASSERT_NE(mkdtemp(dir.data()), nullptr);
		}
#ifdef H5_HAVE_PARALLEL
		MPI_Bcast(dir.data(), dir.size(), MPI_CHAR, 0, MPI_COMM_WORLD);
#endif
		m_dir = m_previous / dir;
		std::filesystem::current_path(m_dir);
	}

	void TearDown() override
	{
		std::filesystem::current_path(m_previous);
#ifdef H5_HAVE_PARALLEL
		MPI_Barrier(MPI_COMM_WORLD);
#endif
		int rank, nprocs;
		world(rank, nprocs);
		if (rank == 0) std::filesystem::remove_all(m_dir);
	}
};

} // namespace

TEST_F(DeclHdf5Examples, simple)
{
	ASSERT_EQ(run("simple"), PDI_OK);
	Dataset size = read("simple.h5", "size");
	EXPECT_EQ(size.dims, (std::vector<hsize_t>{2}));
	EXPECT_EQ(size.values, (std::vector<double>{4, 6}));
	Dataset field = read("simple.h5", "field");
	EXPECT_EQ(field.dims, (std::vector<hsize_t>{6, 8}));
	EXPECT_EQ(field.values, block(2));
}

TEST_F(DeclHdf5Examples, names)
{
	ASSERT_EQ(run("names"), PDI_OK);
	EXPECT_TRUE(exists("output_0.h5", "size"));
	EXPECT_FALSE(exists("output_0.h5", "field_0"));
	EXPECT_EQ(read("output_0.h5", "field_1").values, block(1));
	EXPECT_EQ(read("output_0.h5", "field_2").values, block(2));
}

TEST_F(DeclHdf5Examples, several)
{
	ASSERT_EQ(run("several"), PDI_OK);
	EXPECT_EQ(read("output_0.h5", "field_0").values, block(0));
	EXPECT_FALSE(exists("output_0.h5", "field_1"));
	EXPECT_EQ(read("output_0.h5", "field_2").values, block(2));
	EXPECT_EQ(read("output_0.h5", "field_last").values, block(2));
}

TEST_F(DeclHdf5Examples, event)
{
	ASSERT_EQ(run("event"), PDI_OK);
	for (int iter = 0; iter < 3; ++iter) {
		std::string group = "iter" + std::to_string(iter);
		Dataset i = read("output_0.h5", group + "/iter");
		EXPECT_TRUE(i.dims.empty());
		EXPECT_EQ(i.values, (std::vector<double>{static_cast<double>(iter)}));
		EXPECT_EQ(read("output_0.h5", group + "/field").values, block(iter));
	}
}

TEST_F(DeclHdf5Examples, attributes)
{
	ASSERT_EQ(run("attributes"), PDI_OK);
	for (int iter = 0; iter < 3; ++iter) {
		std::string dataset = "field_" + std::to_string(iter);
		EXPECT_EQ(int_attribute("output_0.h5", dataset, "rank"), 0);
		EXPECT_EQ(int_attribute("output_0.h5", dataset, "iteration"), iter);
	}
}

TEST_F(DeclHdf5Examples, memory_selection)
{
	ASSERT_EQ(run("memory_selection"), PDI_OK);
	Dataset field = read("output_0.h5", "field");
	EXPECT_EQ(field.dims, (std::vector<hsize_t>{4, 6}));
	EXPECT_EQ(field.values, interior(2));
}

TEST_F(DeclHdf5Examples, subsize)
{
	ASSERT_EQ(run("subsize"), PDI_OK);
	Dataset field = read("output_0.h5", "field");
	EXPECT_EQ(field.dims, (std::vector<hsize_t>{4, 6}));
	EXPECT_EQ(field.values, interior(2));
}

TEST_F(DeclHdf5Examples, time_series)
{
	ASSERT_EQ(run("time_series"), PDI_OK);
	Dataset field = read("output_0.h5", "field");
	EXPECT_EQ(field.dims, (std::vector<hsize_t>{3, 4, 6}));
	std::vector<double> expected;
	for (int iter = 0; iter < 3; ++iter) {
		std::vector<double> step = interior(iter);
		expected.insert(expected.end(), step.begin(), step.end());
	}
	EXPECT_EQ(field.values, expected);
}

TEST_F(DeclHdf5Examples, regex)
{
	ASSERT_EQ(run("regex"), PDI_OK);
	for (int iter = 0; iter < 3; ++iter) {
		Dataset field = read("output_0.h5", "field_" + std::to_string(iter));
		EXPECT_EQ(field.type_size, sizeof(float));
		EXPECT_EQ(field.dims, (std::vector<hsize_t>{6, 8}));
		EXPECT_EQ(field.values, block(iter));
	}
}

TEST_F(DeclHdf5Examples, no_overwrite)
{
	ASSERT_EQ(run("no_overwrite"), PDI_OK);
	for (int iter = 0; iter < 3; ++iter) {
		EXPECT_EQ(read("output_0_" + std::to_string(iter) + ".h5", "field").values, block(iter));
	}
	// the second run fails, on the first file
	PDI_errhandler_t previous = PDI_errhandler(PDI_NULL_HANDLER);
	EXPECT_NE(run("no_overwrite"), PDI_OK);
	PDI_errhandler(previous);
}

TEST_F(DeclHdf5Examples, first)
{
	ASSERT_EQ(run("first"), PDI_OK);
	EXPECT_EQ(read("output_0.h5", "field_first").values, block(0));
	EXPECT_EQ(read("output_0.h5", "field_last").values, block(2));
}

TEST_F(DeclHdf5Examples, compression)
{
	ASSERT_EQ(run("compression"), PDI_OK);
	EXPECT_EQ(read("output_0.h5", "field").values, block(2));
	hid_t f = H5Fopen("output_0.h5", H5F_ACC_RDONLY, H5P_DEFAULT);
	hid_t d = H5Dopen2(f, "field", H5P_DEFAULT);
	hid_t plist = H5Dget_create_plist(d);
	hsize_t chunk[2];
	ASSERT_EQ(H5Pget_chunk(plist, 2, chunk), 2);
	EXPECT_EQ(chunk[0], 6u);
	EXPECT_EQ(chunk[1], 4u);
	unsigned flags;
	size_t nelmts = 1;
	unsigned level;
	EXPECT_GE(H5Pget_filter_by_id2(plist, H5Z_FILTER_DEFLATE, &flags, &nelmts, &level, 0, NULL, NULL), 0);
	EXPECT_EQ(level, 6u);
	EXPECT_GE(H5Pget_filter_by_id2(plist, H5Z_FILTER_FLETCHER32, &flags, NULL, NULL, 0, NULL, NULL), 0);
	H5Pclose(plist);
	H5Dclose(d);
	H5Fclose(f);
}

#ifdef H5_HAVE_PARALLEL
// run with 2 processes too, as the page shows
TEST_F(DeclHdf5Examples, parallel)
{
	ASSERT_EQ(run("parallel"), PDI_OK);
	int rank, nprocs;
	world(rank, nprocs);
	MPI_Barrier(MPI_COMM_WORLD);
	if (rank == 0) {
		Dataset field = read("output.h5", "field");
		EXPECT_EQ(field.dims, (std::vector<hsize_t>{static_cast<hsize_t>(nprocs * 4), 6}));
		EXPECT_EQ(field.values, interior(2, nprocs));
	}
}
#endif

TEST_F(DeclHdf5Examples, restart)
{
	ASSERT_EQ(run("checkpoint"), PDI_OK);

	// the restart program
	ASSERT_EQ(PDI_init(PC_parse_path(tree("restart").c_str())), PDI_OK);
	int rank = 0;
	ASSERT_EQ(PDI_expose("rank", &rank, PDI_OUT), PDI_OK);
	int64_t dims[2] = {0, 0};
	ASSERT_EQ(PDI_expose("dims", dims, PDI_INOUT), PDI_OK);
	ASSERT_EQ(dims[0], 6);
	ASSERT_EQ(dims[1], 8);
	std::vector<double> field(dims[0] * dims[1]);
	int iter = -1;
	ASSERT_EQ(PDI_multi_expose("restart", "iter", &iter, PDI_IN, "field", field.data(), PDI_IN, NULL), PDI_OK);
	EXPECT_EQ(iter, 2);
	EXPECT_EQ(field, block(2));
	ASSERT_EQ(PDI_finalize(), PDI_OK);
}

int main(int argc, char* argv[])
{
	::testing::InitGoogleTest(&argc, argv);
#ifdef H5_HAVE_PARALLEL
	MPI_Init(&argc, &argv);
#endif
	int result = RUN_ALL_TESTS();
#ifdef H5_HAVE_PARALLEL
	MPI_Finalize();
#endif
	return result;
}
