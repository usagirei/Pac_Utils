#pragma once

#include "defines.h"

#include <memory>
#include <string>

namespace lib_pac
{
	class EXPORTS file_source_base
	{
	public:
		file_source_base() = default;
		virtual ~file_source_base() = default;
		virtual bool compressed() = 0;
		virtual uint32_t data_size() = 0;
		virtual uint32_t unpacked_size() = 0;

		virtual std::unique_ptr<file_source_base> get_copy() const = 0;

		virtual void copy_data(char* dst, uint32_t offset, uint32_t count) = 0;
	};
}
