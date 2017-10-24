#include "membuf.h"


memory_buffer::memory_buffer(): m_current_size(0), m_data(nullptr)
{
}

void
memory_buffer::reserve(size_t size)
{
	if (!m_data)
	{
		m_data = new char[size];
		return;
	}
	if (m_current_size < size)
	{
		delete[] m_data;
		m_data = new char[size];
		m_current_size = size;
	}
}

char*
memory_buffer::data() const
{
	return m_data;
}

memory_buffer::~memory_buffer()
{
	if (m_data)
		delete[] m_data;
}
