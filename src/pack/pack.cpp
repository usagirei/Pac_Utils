// pack.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

#include <filesystem>
#include <iostream>
#include <iomanip>

#include "pac.h"
#include "pacfilesource.h"
#include "systemfilesource.h"

namespace fs = std::experimental::filesystem;

static fs::path path_make_relative(const fs::path& from, const fs::path& to);
void pack_archive(const fs::path& path);
void report_progress(const lib_pac::pac_archive::progress_info& prog_info);

int
wmain(int argc, const wchar_t** argv)
{
	std::cout << "PAC Packer" << std::endl;
	if (argc == 1)
	{
		std::cout << "Usage: pack.exe <directory>" << std::endl;
		return 1;
	}

	for (int i = 1; i < argc; ++i)
	{
		const std::wstring arg = argv[i];
		fs::path path = arg;
		if (fs::is_directory(path))
			pack_archive(path);
	}
}

void
pack_archive(const fs::path& path)
{
	fs::path root = path.parent_path();
	fs::path base = path.stem();

	fs::path target = path;
	target.replace_extension(".pac");

	if (is_regular_file(target))
	{
		std::cout << "Creating Backup File..." << std::endl;
		fs::path bak_file = target;
		bak_file.replace_extension(".pac.bak");

		if (!is_regular_file(bak_file))
		{
			fs::rename(target, bak_file);
			std::cout << "Backup File Created" << std::endl;
		}
		else
		{
			std::cout << "Backup File Already Exists" << std::endl;
		}
	}

	fs::recursive_directory_iterator iter(path);

	std::cout << "Creating archive: " << target.stem() << std::endl;
	lib_pac::pac_archive archive;
	std::cout << "Aggregating Files..." << std::endl;

	for (auto& it : iter)
	{
		if (fs::is_regular_file(it))
		{
			fs::path virt_path = path_make_relative(path, it);
			auto ptr = std::make_unique<lib_pac::system_file_source>(it.path());
			archive.insert(virt_path.string(), std::move(ptr));
		}
	}

	std::cout << "Found " << archive.num_files() << " Files" << std::endl;
	std::cout << "Compressing..." << std::endl;

	const auto save_info = archive.save(target, report_progress);

	const float ratio = (save_info.compressed_size + save_info.header_size) * 100.f / (save_info.original_size);

	std::cout << "Total Size       : " << save_info.compressed_size + save_info.header_size << std::endl;
	std::cout << "Compression Ratio: " << std::fixed << std::setprecision(2) << ratio << "%" << std::endl;
}

void
report_progress(const lib_pac::pac_archive::progress_info& prog_info)
{
	const int n_digits = ceil(log10(prog_info.num_files));
	const float ratio = prog_info.compressed_size * 100.f / prog_info.raw_size;

	std::cout << "[" << std::setw(n_digits) << prog_info.cur_file << "/" << prog_info.num_files << "] ";
	std::cout << std::fixed << std::setprecision(0) << ratio << "% - " << prog_info.file_name << std::endl;
}

static fs::path
path_make_relative(const fs::path& from, const fs::path& to)
{
	auto from_iter = from.begin();
	auto to_iter = to.begin();

	while (from_iter != from.end() && to_iter != to.end() && (*to_iter) == (*from_iter))
	{
		++to_iter;
		++from_iter;
	}

	fs::path final_path;
	while (from_iter != from.end())
	{
		final_path /= "..";
		++from_iter;
	}

	while (to_iter != to.end())
	{
		final_path /= *to_iter;
		++to_iter;
	}

	return final_path;
}
