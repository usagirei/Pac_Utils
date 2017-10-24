#pragma once

#include "defines.h"

#include <memory>
#include <string>

#include "filesourcebase.h"

namespace lib_pac
{
	class system_file_source : public file_source_base
	{
	private:
		std::wstring m_file;
		uint32_t m_size;

	public:
		EXPORTS ~system_file_source();
		EXPORTS system_file_source(std::wstring system_file);

		EXPORTS bool compressed() override;
		EXPORTS uint32_t data_size() override;
		EXPORTS uint32_t unpacked_size() override;

		EXPORTS std::unique_ptr<file_source_base> get_copy() const override;

		EXPORTS void copy_data(char* dst, uint32_t offset, uint32_t count) override;
	};
}
