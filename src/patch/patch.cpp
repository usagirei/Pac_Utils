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
void patch_archive(const fs::path& path);
void report_progress(const lib_pac::pac_archive::progress_info& prog_info);

int
wmain(int argc, const wchar_t** argv)
{
	std::cout << "PAC Patcher" << std::endl;
	if (argc == 1)
	{
		std::cout << "Usage: patch.exe <directory or pac file>" << std::endl;
		return 1;
	}

	for (int i = 1; i < argc; ++i)
	{
		const std::wstring arg = argv[i];
		fs::path path = arg;
		path.replace_extension();
		if (fs::is_directory(path))
			patch_archive(path);
	}
}

void
patch_archive(const fs::path& path)
{
	fs::path root = path.parent_path();
	fs::path base = path.stem();

	fs::path source = path;
	source.replace_extension();

	fs::path target = path;
	target.replace_extension(".pac");

	fs::path bak_file = target;
	bak_file.replace_extension(".pac.bak");

	if (!is_regular_file(target) && !is_regular_file(bak_file))
	{
		std::cout << "Unable to find PAC file or Backup: " << base;
		return;
	}

	if (!is_regular_file(bak_file))
	{
		std::cout << "Creating Backup File..." << std::endl;
		fs::rename(target, bak_file);
	}

	std::cout << "Reading archive: " << target.stem() << std::endl;
	lib_pac::pac_archive archive(bak_file);
	std::cout << "Replacing Files..." << std::endl;
	fs::recursive_directory_iterator iter(path);

	int n_repl = 0;
	for (auto& it : iter)
	{
		if (fs::is_regular_file(it))
		{
			fs::path virt_path = path_make_relative(path, it);
			if (archive.get(virt_path.string()) != nullptr)
			{
				n_repl++;
				auto ptr = std::make_unique<lib_pac::system_file_source>(it.path());
				archive.insert(virt_path.string(), std::move(ptr));
			}
			else
			{
				std::cout << "File '" << virt_path << "' not found in archive" << std::endl;
			}
		}
	}

	std::cout << "Archive has " << archive.num_files() << " Files" << std::endl;
	std::cout << "Replacing " << n_repl << " File(s)" << std::endl;
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
