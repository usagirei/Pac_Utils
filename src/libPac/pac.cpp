#include "pac.h"
#include <fstream>
#include <algorithm>
#include <memory>

#include "structs.h"
#include "pacfilesource.h"
#include "compressor.h"
#include "membuf.h"

#include <vector>
#include <iostream>
#include <filesystem>

namespace fs = std::experimental::filesystem;

lib_pac::pac_archive::pac_archive(std::wstring path)
	: m_entries()
{
	structs::PAC_HEADER header;
	structs::PAC_DIRECTORY_ENTRY entry;

	std::ifstream file(path, std::ios::binary);

	file.read(reinterpret_cast<char*>(&header), sizeof(header));
	if (strncmp(header.Magic, "DW_PACK", 8) != 0) {
		std::cerr << "Invalid PAC Header" << std::endl;
		return;
	}

	const uint32_t baseOffset = sizeof(header) + header.NumFiles * sizeof(entry);

	for (uint32_t i = 0; i < header.NumFiles; ++i)
	{
		file.read(reinterpret_cast<char*>(&entry), sizeof(entry));
		//std::cout << entry.FileName << std::endl;

		//const auto src = new PacFileSource(path, baseOffset, entry);
		auto ptr = std::make_shared<pac_file_source>(path, baseOffset, entry);
		const std::string f_name = entry.FileName;
		m_entries[f_name] = (std::move(ptr));
	}
}

lib_pac::pac_archive::pac_archive()
	: m_entries()
{
}

lib_pac::pac_archive::archive_info
lib_pac::pac_archive::save(std::wstring file, progress_callback callback) const
{
	std::ofstream output(file, std::ios::binary | std::ios::trunc);

	structs::PAC_HEADER header;
	// DW_PACK\0
	//static char magic[8] = "DW_PACK";
	//strcpy_s(header.Magic, magic);
	header.NumFiles = m_entries.size();

	std::list<std::pair<std::string, std::shared_ptr<file_source_base>>> sorted(m_entries.begin(), m_entries.end());

	sorted.sort([](std::pair<std::string, std::shared_ptr<file_source_base>> a,
	               std::pair<std::string, std::shared_ptr<file_source_base>> b)
	{
		fs::path path_a = a.first;
		fs::path path_b = b.first;

		auto it_a = path_a.begin();
		auto it_b = path_b.begin();

		while (it_a != path_a.end() && it_b != path_a.end())
		{
			fs::path node_a = *it_a;
			fs::path node_b = *it_b;

			// If Same, Check Next
			if (node_a != node_b)
			{
				// If Both are Files, check lexically which one goes first
				if (node_a.has_extension() && node_b.has_extension())
					return node_a < node_b;
				// If either is a directory, and their basenames are the same, directory goes first
				fs::path stem_a = node_a.stem();
				while (stem_a.has_extension())
					stem_a.replace_extension();
				fs::path stem_b = node_b.stem();
				while (stem_b.has_extension())
					stem_b.replace_extension();
				if (stem_a == stem_b)
					return !node_a.has_extension();
				// Else compare the basename lexically
				return stem_a < stem_b;
			}
			++it_a;
			++it_b;
		}
		// I Don't think we should reach here...
		return it_a == path_a.end();
	});

	static const size_t HEADER_SIZE = sizeof(structs::PAC_HEADER);
	static const size_t ENTRY_SIZE = sizeof(structs::PAC_DIRECTORY_ENTRY);

	const size_t data_start = HEADER_SIZE + header.NumFiles * sizeof(structs::PAC_DIRECTORY_ENTRY);
	const size_t header_start = HEADER_SIZE;

	int file_id = 0;
	int file_offset = 0;

	memory_buffer file_buf;
	memory_buffer comp_buf;

	archive_info arch_info;
	arch_info.total_files = header.NumFiles;
	arch_info.header_size = data_start;

	output.write((char*)&header, HEADER_SIZE);

	for (auto pair : sorted)
	{
		auto& file_source = pair.second;

		structs::PAC_DIRECTORY_ENTRY entry;
		entry.FileId = file_id;
		strcpy_s(entry.FileName, pair.first.c_str());
		if (file_source->compressed())
		{
			const uint32_t comp_size = file_source->data_size();
			const uint32_t dec_size = file_source->unpacked_size();

			entry.CompSize = comp_size;
			entry.RawSize = dec_size;
			entry.Compressed = 1;
			entry.Offset = file_offset;

			comp_buf.reserve(comp_size);
			file_source->copy_data(comp_buf.data(), 0, comp_size);
		}
		else
		{
			const uint32_t dec_size = file_source->unpacked_size();
			file_buf.reserve(dec_size);
			file_source->copy_data(file_buf.data(), 0, file_source->unpacked_size());
			const auto comp_info = compressor::prepare_compression(file_buf.data(), dec_size, 0x20000);
			const uint32_t comp_size = comp_info->output_size();

			comp_buf.reserve(comp_size);
			compressor::compress(*comp_info, comp_buf.data());

			entry.CompSize = comp_size;
			entry.RawSize = dec_size;
			entry.Compressed = 1;
			entry.Offset = file_offset;
		}

		output.seekp(header_start + ENTRY_SIZE * file_id);
		output.write((char*)&entry, ENTRY_SIZE);

		output.seekp(data_start + file_offset);
		output.write((char*)comp_buf.data(), entry.CompSize);

		file_id++;
		file_offset += entry.CompSize;


		if (callback)
		{
			const progress_info info(file_id, header.NumFiles, pair.first, entry.RawSize, entry.CompSize);
			callback(info);
		}
		arch_info.compressed_size += entry.CompSize;
		arch_info.original_size += entry.RawSize;
	}

	return arch_info;
}

size_t
lib_pac::pac_archive::num_files() const
{
	return m_entries.size();
}


bool
lib_pac::pac_archive::remove(const std::string& file)
{
	auto found = m_entries.find(file);
	if (found == m_entries.end())
		return false;
	m_entries.erase(file);
	return true;
}

std::shared_ptr<lib_pac::file_source_base>
lib_pac::pac_archive::get(const std::string& file)
{
	auto found = m_entries.find(file);
	if (found == m_entries.end())
		return std::shared_ptr<file_source_base>(nullptr);
	return found->second;
}

void
lib_pac::pac_archive::insert(const std::string& virt_path, std::shared_ptr<file_source_base> src)
{
	auto found = m_entries.find(virt_path);
	if (found != m_entries.end())
		m_entries.erase(virt_path);
	m_entries[virt_path] = std::move(src);
}

// Iterator

lib_pac::pac_archive::iterator
lib_pac::pac_archive::begin()
{
	auto begin = m_entries.begin();
	return iterator(begin);
}

lib_pac::pac_archive::iterator
lib_pac::pac_archive::end()
{
	auto end = m_entries.end();
	return iterator(end);
}


lib_pac::pac_archive::iterator::iterator(std::map<std::string, std::shared_ptr<file_source_base>>::iterator& base)
{
	m_iter = base;
}

std::string
lib_pac::pac_archive::iterator::operator*() const
{
	return (*m_iter).first;
}

lib_pac::pac_archive::iterator&
lib_pac::pac_archive::iterator::operator++()
{
	++m_iter;
	return *this;
}

bool
lib_pac::pac_archive::iterator::operator!=(const iterator& rhs) const
{
	return m_iter != rhs.m_iter;
}

lib_pac::pac_archive::progress_info::progress_info(int cur_file, int num_files, const std::string& file_name,
                                                   uint32_t raw_size, uint32_t compressed_size): cur_file(cur_file),
                                                                                                 num_files(num_files),
                                                                                                 file_name(file_name),
                                                                                                 raw_size(raw_size),
                                                                                                 compressed_size(
	                                                                                                 compressed_size)
{
}
