#pragma once
#include "defines.h"

#include <vector>

namespace lib_pac
{
	class bit_writer
	{
	private:
		uint8_t m_remainder_bits;
		size_t m_offset;
		uint8_t* m_data;
		void write_byte(uint8_t data, uint8_t n_bits);

	public:
		EXPORTS explicit bit_writer(void* data);

		EXPORTS void write_bit(bool one);
		EXPORTS void write_bits(uint32_t data, uint8_t n_bits);
		
		EXPORTS void seek(size_t offset, uint8_t bit);
		EXPORTS size_t tell(uint8_t* bit = nullptr) const;

	};

	class bit_reader
	{
	private:
		uint8_t m_remainder_bits;
		size_t m_offset;
		const uint8_t* m_data;
	public:
		EXPORTS explicit bit_reader(const void* data);

		EXPORTS uint8_t read_byte();
		EXPORTS bool read_bit();

		EXPORTS void seek(size_t offset, uint8_t bit);
	};
}
