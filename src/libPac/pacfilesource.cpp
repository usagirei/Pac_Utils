#include <algorithm>
#include <fstream>

#include "structs.h"
#include "pacfilesource.h"

lib_pac::pac_file_source::~pac_file_source()
{
}

lib_pac::pac_file_source::pac_file_source(std::wstring file, uint32_t base_offset,
                                     lib_pac::structs::PAC_DIRECTORY_ENTRY& entry) :
	m_pac_file(file),
	m_offset(base_offset + entry.Offset),
	m_dec_size(entry.RawSize),
	m_comp_size(entry.CompSize),
	m_compressed(entry.Compressed)
{
}

bool lib_pac::pac_file_source::compressed()
{
	return m_compressed;
}

size_t lib_pac::pac_file_source::data_size()
{
	return m_comp_size;
}

uint32_t lib_pac::pac_file_source::unpacked_size()
{
	return m_dec_size;
}

std::unique_ptr<lib_pac::file_source_base> lib_pac::pac_file_source::get_copy() const
{
	return std::make_unique<pac_file_source>(*this);
}

void lib_pac::pac_file_source::copy_data(char* dst, uint32_t offset, uint32_t count)
{
	std::ifstream file(m_pac_file, std::ios::binary);
	file.seekg(m_offset + offset, std::ios_base::beg);
	uint32_t to_read = std::min(count, m_comp_size - offset);

	file.read(dst, to_read);
}
