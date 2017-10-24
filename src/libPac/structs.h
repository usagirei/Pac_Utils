#pragma once

#include <stdint.h>

namespace  lib_pac
{
	namespace structs
	{
		struct PAC_HEADER
		{
			char Magic[8] {"DW_PACK"};
			uint32_t __Zero0 = 0;
			uint32_t NumFiles = -1;
			uint32_t __Zero1 = 0;
		};

		struct PAC_DIRECTORY_ENTRY
		{
			uint32_t __Zero0 = 0;
			uint32_t FileId = -1;
			char FileName[260] {0};
			uint32_t __Zero1 = 0;
			uint32_t CompSize = -1;
			uint32_t RawSize = -1;
			uint32_t Compressed = -1;
			uint32_t Offset = -1;
		};
	}
}