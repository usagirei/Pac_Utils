#include "bitstream.h"
#include <algorithm>


lib_pac::bit_writer::bit_writer(void* data)
	: m_remainder_bits(8), m_offset(0), m_data(static_cast<uint8_t*>(data))
{
}

void lib_pac::bit_writer::seek(size_t offset, uint8_t bit)
{
	m_offset = offset;
	m_remainder_bits = 8 - bit;
}

size_t lib_pac::bit_writer::tell(uint8_t * bit) const
{
	if (bit)
		*bit = 8 - m_remainder_bits;
	return m_offset;
}

void lib_pac::bit_writer::write_bit(bool high)
{
	if (m_remainder_bits == 0)
	{
		m_offset++;
		m_remainder_bits = 8;
	}
	m_remainder_bits--;

	uint8_t curr = m_data[m_offset];
	if (high)
		curr |= (1 << m_remainder_bits);
	else
		curr &= ~(1 << m_remainder_bits);
	m_data[m_offset] = curr;
}

// TODO: Aligned Writes
void lib_pac::bit_writer::write_bits(uint32_t data, uint8_t nBits)
{
	nBits = std::min((uint8_t)32, nBits);

	while(nBits > 8)
	{
		nBits -= 8;
		write_byte(data >> nBits, 8);
	}

	write_byte(data, nBits);
}


void lib_pac::bit_writer::write_byte(uint8_t data, uint8_t nBits)
{
	nBits = std::min((uint8_t)8, nBits);

	if (nBits == 0)
		return;

	if (m_remainder_bits == 0)
	{
		m_offset++;
		m_remainder_bits = 8;
	}

	int first = std::min(nBits, m_remainder_bits);
	int second = nBits - first;

	uint8_t mask, shift;
	{
		mask = (1 << first) - 1;
		shift = (m_remainder_bits - first);

		uint8_t curr = m_data[m_offset];
		curr &= ~(mask << shift);

		uint8_t data0 = data >> second;
		data0 &= mask;
		data0 <<= shift;
		curr |= data0;

		m_data[m_offset] = curr;
		m_remainder_bits -= first;
	}
	if (second)
	{
		m_offset++;
		m_remainder_bits = (8 - second);

		mask = (1 << second) - 1;
		shift = m_remainder_bits;

		uint8_t curr = m_data[m_offset];
		curr &= ~(mask << shift);

		uint8_t data1 = data << shift;
		data1 &= mask << shift;
		curr |= data1;

		m_data[m_offset] = curr;
	}
}


lib_pac::bit_reader::bit_reader(const void* data)
	: m_remainder_bits(8), m_offset(0), m_data(static_cast<const uint8_t*>(data))
{
}

void lib_pac::bit_reader::seek(size_t offset, uint8_t bit)
{
	m_offset = offset;
	m_remainder_bits = 8 - bit;
}


bool lib_pac::bit_reader::read_bit()
{
	if (m_remainder_bits == 0)
	{
		m_offset++;
		m_remainder_bits = 8;
	}
	return ((m_data[m_offset] >> --m_remainder_bits) & 0x01) == 0x01;
}


uint8_t lib_pac::bit_reader::read_byte()
{
	if (m_remainder_bits == 8)
		return m_data[m_offset++];
	if (m_remainder_bits == 0)
		return m_data[++m_offset];

	const int first = std::min(static_cast<uint8_t>(8), m_remainder_bits);
	const int second = 8 - first;

	uint8_t value = 0;

	uint8_t data0 = m_data[m_offset];
	data0 &= (1 << first) - 1;
	data0 <<= second;
	value |= data0;

	m_offset++;

	uint8_t data1 = m_data[m_offset];
	data1 &= (1 << second) - 1 << first;
	data1 >>= first;
	value |= data1;

	return value;
}
