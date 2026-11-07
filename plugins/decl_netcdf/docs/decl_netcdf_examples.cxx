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
/// Runs the specification trees of the Decl'NetCDF examples page
/// (plugins/decl_netcdf/examples.md) with the programs it describes, and checks
/// the files they write against what the page says of them.

#include <filesystem>
#include <string>
#include <vector>

#include <stdlib.h>

#include <gtest/gtest.h>
#include <netcdf.h>
#ifdef EXAMPLES_MPI
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
#ifdef EXAMPLES_MPI
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	MPI_Comm_size(MPI_COMM_WORLD, &nprocs);
#endif
}

/// The value of a cell of field: 100 * iter + 10 * its global row + its column, -1 for the ghost cells
std::vector<double> block(int iter, int rank = 0)
{
	std::vector<double> result;
	for (int i = 0; i < 4 + 2; ++i) {
		for (int j = 0; j < 6 + 2; ++j) {
			bool ghost = (i == 0 || i == 4 + 1 || j == 0 || j == 6 + 1);
			result.push_back(ghost ? -1 : 100 * iter + 10 * (rank * 4 + i - 1) + j - 1);
		}
	}
	return result;
}

/// The program that the page describes: it shares rank, nprocs and size, then iter and field at each of 3 iterations
void run(const std::string& name)
{
	ASSERT_EQ(PDI_init(PC_parse_path(tree(name).c_str())), PDI_OK);
	int rank, nprocs;
	world(rank, nprocs);
	int size[2] = {4, 6};
	ASSERT_EQ(PDI_multi_expose("init", "rank", &rank, PDI_OUT, "nprocs", &nprocs, PDI_OUT, "size", size, PDI_OUT, NULL), PDI_OK);
	for (int iter = 0; iter < 3; ++iter) {
		std::vector<double> field = block(iter, rank);
		ASSERT_EQ(PDI_multi_expose("iteration", "iter", &iter, PDI_OUT, "field", field.data(), PDI_OUT, NULL), PDI_OK);
	}
	ASSERT_EQ(PDI_finalize(), PDI_OK);
}

/// The id of the group of a path, and the name of the variable in it
int group(int ncid, std::string& path)
{
	for (auto slash = path.find('/'); slash != std::string::npos; slash = path.find('/')) {
		int grp;
		EXPECT_EQ(nc_inq_grp_ncid(ncid, path.substr(0, slash).c_str(), &grp), NC_NOERR) << path;
		ncid = grp;
		path = path.substr(slash + 1);
	}
	return ncid;
}

/// A variable of a file: its dimensions, by name and length, and its values, read as double
struct Variable {
	std::vector<std::string> dim_names;
	std::vector<size_t> dims;
	std::vector<double> values;
};

Variable read(const std::string& file, std::string path)
{
	Variable result;
	int ncid, varid, ndims;
	EXPECT_EQ(nc_open(file.c_str(), NC_NOWRITE, &ncid), NC_NOERR) << file;
	int grp = group(ncid, path);
	EXPECT_EQ(nc_inq_varid(grp, path.c_str(), &varid), NC_NOERR) << file << ":" << path;
	nc_inq_varndims(grp, varid, &ndims);
	std::vector<int> dimids(ndims);
	nc_inq_vardimid(grp, varid, dimids.data());
	size_t npoints = 1;
	for (int dimid: dimids) {
		char name[NC_MAX_NAME + 1];
		size_t len;
		nc_inq_dim(grp, dimid, name, &len);
		result.dim_names.push_back(name);
		result.dims.push_back(len);
		npoints *= len;
	}
	result.values.resize(npoints);
	nc_get_var_double(grp, varid, result.values.data());
	nc_close(ncid);
	return result;
}

bool exists(const std::string& file, const std::string& name)
{
	int ncid, varid;
	EXPECT_EQ(nc_open(file.c_str(), NC_NOWRITE, &ncid), NC_NOERR) << file;
	bool result = nc_inq_varid(ncid, name.c_str(), &varid) == NC_NOERR;
	nc_close(ncid);
	return result;
}

/// An integer attribute of a group (empty variable) or of a variable
int int_attribute(const std::string& file, std::string group_path, const std::string& variable, const std::string& name)
{
	int ncid, varid = NC_GLOBAL, result = -1;
	EXPECT_EQ(nc_open(file.c_str(), NC_NOWRITE, &ncid), NC_NOERR) << file;
	group_path += "/";
	int grp = group(ncid, group_path);
	if (!variable.empty()) EXPECT_EQ(nc_inq_varid(grp, variable.c_str(), &varid), NC_NOERR);
	EXPECT_EQ(nc_get_att_int(grp, varid, name.c_str(), &result), NC_NOERR) << name;
	nc_close(ncid);
	return result;
}

/// Each example runs in a directory of its own, removed afterwards
class DeclNetcdfExamples: public ::testing::Test
{
	std::filesystem::path m_previous;
	std::filesystem::path m_dir;

protected:
	void SetUp() override
	{
		m_previous = std::filesystem::current_path();
		int rank, nprocs;
		world(rank, nprocs);
		std::string dir = "decl_netcdf_examples-XXXXXX";
		if (rank == 0) {
			ASSERT_NE(mkdtemp(dir.data()), nullptr);
		}
#ifdef EXAMPLES_MPI
		MPI_Bcast(dir.data(), dir.size(), MPI_CHAR, 0, MPI_COMM_WORLD);
#endif
		m_dir = m_previous / dir;
		std::filesystem::current_path(m_dir);
	}

	void TearDown() override
	{
		std::filesystem::current_path(m_previous);
#ifdef EXAMPLES_MPI
		MPI_Barrier(MPI_COMM_WORLD);
#endif
		int rank, nprocs;
		world(rank, nprocs);
		if (rank == 0) std::filesystem::remove_all(m_dir);
	}
};

} // namespace

TEST_F(DeclNetcdfExamples, simple)
{
	run("simple");
	Variable size = read("simple.nc", "size");
	EXPECT_EQ(size.dim_names, (std::vector<std::string>{"size_0"}));
	EXPECT_EQ(size.values, (std::vector<double>{4, 6}));
	Variable field = read("simple.nc", "field");
	EXPECT_EQ(field.dim_names, (std::vector<std::string>{"field_0", "field_1"}));
	EXPECT_EQ(field.dims, (std::vector<size_t>{6, 8}));
	EXPECT_EQ(field.values, block(2));
}

TEST_F(DeclNetcdfExamples, names)
{
	run("names");
	EXPECT_TRUE(exists("output_0.nc", "size"));
	EXPECT_FALSE(exists("output_0.nc", "field_0"));
	EXPECT_EQ(read("output_0.nc", "field_1").values, block(1));
	EXPECT_EQ(read("output_0.nc", "field_2").values, block(2));
}

TEST_F(DeclNetcdfExamples, event)
{
	run("event");
	for (int iter = 0; iter < 3; ++iter) {
		std::string group = "iter" + std::to_string(iter);
		Variable i = read("output_0.nc", group + "/iter");
		EXPECT_TRUE(i.dims.empty());
		EXPECT_EQ(i.values, (std::vector<double>{static_cast<double>(iter)}));
		EXPECT_EQ(read("output_0.nc", group + "/field").values, block(iter));
	}
}

TEST_F(DeclNetcdfExamples, dimensions)
{
	run("dimensions");
	EXPECT_EQ(int_attribute("output_0.nc", "results", "", "nprocs"), 1);
	EXPECT_EQ(int_attribute("output_0.nc", "results", "", "rank"), 0);
	EXPECT_EQ(int_attribute("output_0.nc", "results", "field", "iteration"), 2);
	Variable field = read("output_0.nc", "results/field");
	EXPECT_EQ(field.dim_names, (std::vector<std::string>{"height", "width"}));
	EXPECT_EQ(field.dims, (std::vector<size_t>{6, 8}));
	EXPECT_EQ(field.values, block(2));
	int ncid, grp, varid;
	ASSERT_EQ(nc_open("output_0.nc", NC_NOWRITE, &ncid), NC_NOERR);
	ASSERT_EQ(nc_inq_grp_ncid(ncid, "results", &grp), NC_NOERR);
	ASSERT_EQ(nc_inq_varid(grp, "field", &varid), NC_NOERR);
	char units[16] = {};
	size_t len;
	ASSERT_EQ(nc_inq_attlen(grp, varid, "units", &len), NC_NOERR);
	ASSERT_LT(len, sizeof(units));
	ASSERT_EQ(nc_get_att_text(grp, varid, "units", units), NC_NOERR);
	EXPECT_STREQ(units, "kelvin");
	nc_close(ncid);
}

namespace {

struct particle {
	int id;
	double position[3];
};

} // namespace

TEST_F(DeclNetcdfExamples, records)
{
	// the program of the records example
	ASSERT_EQ(PDI_init(PC_parse_path(tree("particles").c_str())), PDI_OK);
	particle particles[2] = {{1, {0.5, 1.5, 2.5}}, {2, {3., 4., 5.}}};
	double time = 0.25;
	ASSERT_EQ(PDI_multi_expose("save", "time", &time, PDI_OUT, "particles", particles, PDI_OUT, NULL), PDI_OK);
	particle loaded[2] = {};
	double loaded_time = 0;
	ASSERT_EQ(PDI_multi_expose("load", "time", &loaded_time, PDI_INOUT, "particles", loaded, PDI_IN, NULL), PDI_OK);
	ASSERT_EQ(PDI_finalize(), PDI_OK);
	EXPECT_EQ(loaded_time, 0.25);
	for (int p = 0; p < 2; ++p) {
		EXPECT_EQ(loaded[p].id, particles[p].id);
		for (int d = 0; d < 3; ++d)
			EXPECT_EQ(loaded[p].position[d], particles[p].position[d]);
	}

	int ncid, varid;
	ASSERT_EQ(nc_open("particles.nc", NC_NOWRITE, &ncid), NC_NOERR);
	ASSERT_EQ(nc_inq_varid(ncid, "particles", &varid), NC_NOERR);
	nc_type type;
	ASSERT_EQ(nc_inq_vartype(ncid, varid, &type), NC_NOERR);
	char name[NC_MAX_NAME + 1];
	size_t size, nfields;
	ASSERT_EQ(nc_inq_compound(ncid, type, name, &size, &nfields), NC_NOERR);
	EXPECT_STREQ(name, "particle_t");
	EXPECT_EQ(nfields, 2u);
	double file_time = 0;
	EXPECT_EQ(nc_get_att_double(ncid, varid, "time", &file_time), NC_NOERR);
	EXPECT_EQ(file_time, 0.25);
	nc_close(ncid);
}

TEST_F(DeclNetcdfExamples, time_series)
{
	run("history");
	int ncid, unlimited;
	ASSERT_EQ(nc_open("history_0.nc", NC_NOWRITE, &ncid), NC_NOERR);
	ASSERT_EQ(nc_inq_unlimdim(ncid, &unlimited), NC_NOERR);
	char name[NC_MAX_NAME + 1];
	nc_inq_dimname(ncid, unlimited, name);
	EXPECT_STREQ(name, "time");
	nc_close(ncid);
	Variable field = read("history_0.nc", "field");
	EXPECT_EQ(field.dim_names, (std::vector<std::string>{"time", "height", "width"}));
	EXPECT_EQ(field.dims, (std::vector<size_t>{3, 6, 8}));
	std::vector<double> expected;
	for (int iter = 0; iter < 3; ++iter) {
		std::vector<double> step = block(iter);
		expected.insert(expected.end(), step.begin(), step.end());
	}
	EXPECT_EQ(field.values, expected);
}

#ifdef EXAMPLES_MPI
// run with 2 processes too, as the page shows
TEST_F(DeclNetcdfExamples, parallel)
{
	run("parallel");
	int rank, nprocs;
	world(rank, nprocs);
	MPI_Barrier(MPI_COMM_WORLD);
	if (rank == 0) {
		Variable field = read("output.nc", "field");
		EXPECT_EQ(field.dims, (std::vector<size_t>{static_cast<size_t>(nprocs * 6), 8}));
		std::vector<double> expected;
		for (int r = 0; r < nprocs; ++r) {
			std::vector<double> b = block(2, r);
			expected.insert(expected.end(), b.begin(), b.end());
		}
		EXPECT_EQ(field.values, expected);
	}
}
#endif

TEST_F(DeclNetcdfExamples, compression)
{
	run("compression");
	EXPECT_EQ(read("output_0.nc", "field").values, block(2));
	int ncid, varid, storage, shuffle, deflate, level;
	size_t chunks[2];
	ASSERT_EQ(nc_open("output_0.nc", NC_NOWRITE, &ncid), NC_NOERR);
	ASSERT_EQ(nc_inq_varid(ncid, "field", &varid), NC_NOERR);
	ASSERT_EQ(nc_inq_var_chunking(ncid, varid, &storage, chunks), NC_NOERR);
	EXPECT_EQ(storage, NC_CHUNKED);
	EXPECT_EQ(chunks[0], 6u);
	EXPECT_EQ(chunks[1], 4u);
	ASSERT_EQ(nc_inq_var_deflate(ncid, varid, &shuffle, &deflate, &level), NC_NOERR);
	EXPECT_TRUE(deflate);
	EXPECT_EQ(level, 6);
	nc_close(ncid);
}

TEST_F(DeclNetcdfExamples, restart)
{
	run("history");

	// the restart program
	ASSERT_EQ(PDI_init(PC_parse_path(tree("restart").c_str())), PDI_OK);
	int rank = 0;
	ASSERT_EQ(PDI_expose("rank", &rank, PDI_OUT), PDI_OK);
	int dims[3] = {0, 0, 0};
	ASSERT_EQ(PDI_expose("dims", dims, PDI_INOUT), PDI_OK);
	EXPECT_EQ(dims[0], 3);
	ASSERT_EQ(dims[1], 6);
	ASSERT_EQ(dims[2], 8);
	std::vector<double> field(dims[1] * dims[2]);
	ASSERT_EQ(PDI_expose("field", field.data(), PDI_IN), PDI_OK);
	EXPECT_EQ(field, block(2));
	ASSERT_EQ(PDI_finalize(), PDI_OK);
}

int main(int argc, char* argv[])
{
	::testing::InitGoogleTest(&argc, argv);
#ifdef EXAMPLES_MPI
	MPI_Init(&argc, &argv);
#endif
	int result = RUN_ALL_TESTS();
#ifdef EXAMPLES_MPI
	MPI_Finalize();
#endif
	return result;
}
