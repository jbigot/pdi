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
/// The POSIX plugin of the plugin creation tutorial (How_to_create_a_plugin.md), shown there one snippet at a time.

//! [parse]
#include <filesystem>
#include <fstream>
#include <memory>
#include <string>
#include <unordered_map>

#include <pdi/context.h>
#include <pdi/error.h>
#include <pdi/expression.h>
#include <pdi/logger.h>
#include <pdi/paraconf_wrapper.h>
#include <pdi/plugin.h>
#include <pdi/ref_any.h>

namespace {

/// A file to access, and the condition to access it
struct File_op {
	PDI::Expression file;
	PDI::Expression when = 1L;
};

/// Parses a FILE_OP: either the file name alone, or a map with `file' and optionally `when'
File_op parse_file_op(PC_tree_t tree)
{
	if (PDI::is_scalar(tree)) {
		return {PDI::to_string(tree)};
	}
	File_op result;
	PDI::each(tree, [&](PC_tree_t key_tree, PC_tree_t value) {
		std::string key = PDI::to_string(key_tree);
		if (key == "file") {
			result.file = PDI::to_string(value);
		} else if (key == "when") {
			result.when = PDI::to_string(value);
		} else {
			throw PDI::Spectree_error{key_tree, "Unknown key in POSIX file operation: `{}'", key};
		}
	});
	if (PC_status(PC_get(tree, ".file"))) {
		throw PDI::Spectree_error{tree, "Missing `file' in POSIX file operation"};
	}
	return result;
}

} // namespace

//! [parse]

//! [constructor]
class posix_plugin: public PDI::Plugin
{
	/// data name -> where and when to write it
	std::unordered_map<std::string, File_op> m_writes;

	/// data name -> where and when to read it
	std::unordered_map<std::string, File_op> m_reads;

	/// data name -> the file whose existence to store in it
	std::unordered_map<std::string, PDI::Expression> m_exists;

public:
	posix_plugin(PDI::Logger& logger, PDI::Context& ctx, PC_tree_t spec_tree)
		: Plugin{logger, ctx}
	{
		PDI::each(spec_tree, [&](PC_tree_t key_tree, PC_tree_t value) {
			std::string key = PDI::to_string(key_tree);
			if (key == "write") {
				PDI::each(value, [&](PC_tree_t data, PC_tree_t op) { m_writes.emplace(PDI::to_string(data), parse_file_op(op)); });
			} else if (key == "read") {
				PDI::each(value, [&](PC_tree_t data, PC_tree_t op) { m_reads.emplace(PDI::to_string(data), parse_file_op(op)); });
			} else if (key == "exists") {
				PDI::each(value, [&](PC_tree_t data, PC_tree_t file) { m_exists.emplace(PDI::to_string(data), PDI::to_string(file)); });
			} else if (key != "logging") { // `logging' is handled by PDI
				throw PDI::Spectree_error{key_tree, "Unknown key in POSIX plugin configuration: `{}'", key};
			}
		});
		ctx.on_data([this](const std::string& name, PDI::Ref ref) { this->data(name, ref); });
	}

	//! [constructor]

private:
	//! [data]
	void data(const std::string& name, PDI::Ref ref)
	{
		// read before writing, so that a data both read and written on the same share is not overwritten before it is read
		if (auto&& op = m_reads.find(name); op != m_reads.end() && op->second.when.to_long(context())) {
			read(name, ref, op->second.file.to_string(context()));
		}
		if (auto&& op = m_writes.find(name); op != m_writes.end() && op->second.when.to_long(context())) {
			write(name, ref, op->second.file.to_string(context()));
		}
		if (auto&& file = m_exists.find(name); file != m_exists.end()) {
			exists(name, ref, file->second.to_string(context()));
		}
	}

	//! [data]

	//! [write]
	void write(const std::string& name, PDI::Ref_r ref, const std::string& path)
	{
		if (!ref) {
			throw PDI::Permission_error{"Cannot read `{}' to write it to `{}'", name, path};
		}
		auto&& type = ref.type();
		const char* data = static_cast<const char*>(ref.get());
		std::unique_ptr<char[]> dense_copy;
		if (type->buffersize() != type->datasize()) {
			// the data is sparse in memory, make a dense copy of it
			dense_copy.reset(new char[type->datasize()]);
			type->data_to_dense_copy(dense_copy.get(), data);
			data = dense_copy.get();
		}

		// write to a temporary file, then replace the old one, so that a failed write keeps it
		std::string tmp_path = path + ".tmp";
		std::ofstream file{tmp_path, std::ios::binary};
		file.write(data, type->datasize());
		file.close();
		if (!file) {
			throw PDI::System_error{"Cannot write `{}' to `{}'", name, tmp_path};
		}
		std::error_code error;
		std::filesystem::rename(tmp_path, path, error);
		if (error) {
			throw PDI::System_error{"Cannot rename `{}' to `{}': {}", tmp_path, path, error.message()};
		}
	}

	//! [write]

	//! [read]
	void read(const std::string& name, PDI::Ref_w ref, const std::string& path)
	{
		if (!ref) {
			throw PDI::Permission_error{"Cannot write `{}' to read it from `{}'", name, path};
		}
		auto&& type = ref.type();
		std::error_code error;
		auto file_size = std::filesystem::file_size(path, error);
		if (error) {
			throw PDI::System_error{"Cannot read `{}' from `{}': {}", name, path, error.message()};
		}
		if (file_size != type->datasize()) {
			throw PDI::Value_error{"`{}' holds {} B, `{}' needs {} B", path, file_size, name, type->datasize()};
		}

		std::unique_ptr<char[]> dense_copy{new char[type->datasize()]};
		std::ifstream file{path, std::ios::binary};
		file.read(dense_copy.get(), type->datasize());
		if (!file) {
			throw PDI::System_error{"Cannot read `{}' from `{}'", name, path};
		}
		// the data may be sparse in memory, copy it to its layout
		type->data_from_dense_copy(ref.get(), dense_copy.get());
	}

	//! [read]

	//! [exists]
	void exists(const std::string& name, PDI::Ref_w ref, const std::string& path)
	{
		if (!ref) {
			throw PDI::Permission_error{"Cannot write to `{}' whether `{}' exists", name, path};
		}
		// assigns the value according to the type of the data, and throws if it is not a scalar
		ref.scalar_assign(std::filesystem::exists(path) ? 1 : 0);
	}

	//! [exists]
};

PDI_PLUGIN(posix)
