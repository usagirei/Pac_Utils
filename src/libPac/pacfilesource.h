#pragma once

#include "defines.h"

#include <memory>
#include <string>

#include "structs.h"
#include "filesourcebase.h"

namespace lib_pac
{
	class pac_file_source : public file_source_base
	{
	private:
		std::wstring m_pac_file;
		uint32_t m_offset;
		uint32_t m_dec_size;
		uint32_t m_comp_size;
		bool m_compressed;

	public:
		~pac_file_source();
		pac_file_source(std::wstring pac_file, uint32_t base_offset, structs::PAC_DIRECTORY_ENTRY &entry);

		bool compressed() override;
		uint32_t data_size() override;
		uint32_t unpacked_size() override;

		std::unique_ptr<file_source_base> get_copy() const override;

		void copy_data(char* dst, uint32_t offset, uint32_t count) override;
	};

}
