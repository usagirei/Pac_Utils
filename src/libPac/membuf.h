#pragma once
#include "defines.h"

#include <cstdint>

class memory_buffer
{
private:
	size_t m_current_size;
	char* m_data;
public:
	EXPORTS memory_buffer();
	EXPORTS ~memory_buffer();
	EXPORTS void reserve(size_t size);
	EXPORTS char* data() const;
};
