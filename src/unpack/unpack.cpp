// unpack.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

#include <filesystem>
#include <iostream>
#include <fstream>
#include <iomanip>
#include <future>

#include "pac.h"
#include "membuf.h""
#include "compressor.h"

namespace fs = std::experimental::filesystem;

void extract_archive(fs::path path);

int
wmain(int argc, const wchar_t** argv)
{
	std::cout << "PAC Unpacker" << std::endl;
	if (argc == 1)
	{
		std::cout << "Usage: unpack.exe <pac file>" << std::endl;
		return 1;
	}

	for (int i = 1; i < argc; ++i)
	{
		const std::wstring arg = argv[i];
		const fs::path path = arg;
		if (fs::is_regular_file(path))
			extract_archive(path);
	}
}

void
extract_archive(fs::path path)
{
	memory_buffer comp_buffer;
	memory_buffer dec_buffer;

	std::wcout << L"Extracting Archive: " << path.filename() << std::endl;
	lib_pac::pac_archive archive(path);


	const int n_files = archive.num_files();
	const int f_digits = ceil(log10(n_files));

	int cur_file = 0;
	
	for (auto& elem : archive)
	{
		const fs::path v_path = path.stem().append(elem);
		auto file_source = archive.get(elem);

		size_t dec_sz;

		std::cout << "[" << std::setw(f_digits) << cur_file + 1 << "/" << n_files << "] " << v_path << std::endl;
		if (file_source->compressed())
		{
			comp_buffer.reserve(file_source->data_size());

			file_source->copy_data(comp_buffer.data(), 0, file_source->data_size());

			const auto dec_info = lib_pac::compressor::prepare_decompression(comp_buffer.data(), file_source->data_size());

			dec_sz = dec_info->output_size();
			const uint32_t expected_size = file_source->unpacked_size();

			if (dec_sz != expected_size)
			{
				std::cerr << "Size Mismatch: " << v_path;
				std::cerr << " - Expected " << expected_size << " got " << dec_sz << std::endl;
			}

			dec_buffer.reserve(dec_sz);

			lib_pac::compressor::decompress(*dec_info, dec_buffer.data());
		}
		else
		{
			dec_sz = file_source->unpacked_size();
			dec_buffer.reserve(dec_sz);

			file_source->copy_data(dec_buffer.data(), 0, dec_sz);
		}
		fs::create_directories(v_path.parent_path());

#if 0
		HANDLE hFile = CreateFileA(v_path.string().c_str(), GENERIC_WRITE, 0, nullptr, CREATE_NEW,
			FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN | FILE_FLAG_WRITE_THROUGH,
			nullptr);
		
		WriteFile(hFile, dec_buffer.data(), dec_sz, nullptr, nullptr);
		CloseHandle(hFile);
#endif
#if 0
		std::ofstream output(v_path, std::ios::trunc | std::ios::binary);
		output.write(dec_buffer.data(), dec_sz);
		output.flush();
		output.close();
#endif
#if 1
		FILE* f;
		fopen_s(&f, v_path.string().c_str(), "wb");
		fwrite(dec_buffer.data(), 1, dec_sz, f);
		fflush(f);
		fclose(f);
#endif

		cur_file++;
	}
}
