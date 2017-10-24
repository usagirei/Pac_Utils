#include <algorithm>
#include <fstream>
#include <filesystem>

namespace fs = std::experimental::filesystem;

#include "systemfilesource.h"


lib_pac::system_file_source::~system_file_source()
{
}

lib_pac::system_file_source::system_file_source(std::wstring system_file)
	: m_file(system_file)
{
	m_size = fs::file_size(system_file);;
}

bool lib_pac::system_file_source::compressed()
{
	return false;
}

uint32_t lib_pac::system_file_source::data_size()
{
	return m_size;
}

uint32_t lib_pac::system_file_source::unpacked_size()
{
	return m_size;
}

std::unique_ptr<lib_pac::file_source_base> lib_pac::system_file_source::get_copy() const
{
	return std::make_unique<system_file_source>(*this);
}

void lib_pac::system_file_source::copy_data(char* dst, uint32_t offset, uint32_t count)
{
	std::ifstream file(m_file, std::ios::binary);
	file.seekg(offset, std::ios_base::beg);
	const uint32_t to_read = std::min(count, m_size - offset);

	file.read(dst, to_read);
}
