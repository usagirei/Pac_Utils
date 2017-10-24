#include "stdafx.h"
#include "CppUnitTest.h"
#include <bitstream.h>

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace libPac_Test
{		
	TEST_CLASS(BitReaderTests)
	{
	public:
		
		TEST_METHOD(BitReader_ReadBytes)
		{
			uint8_t data[] = { 0xFF, 0x7F, 0x64 };
			lib_pac::bit_reader reader(data);

			Assert::AreEqual(reader.read_byte(), (uint8_t) 0xFF);
			Assert::AreEqual(reader.read_byte(), (uint8_t) 0x7F);
			Assert::AreEqual(reader.read_byte(), (uint8_t) 0x64);
		}

		TEST_METHOD(BitReader_ReadBits)
		{
			uint8_t data[] = { 0b01001101 };
			lib_pac::bit_reader reader(data);

			Assert::AreEqual(reader.read_bit(), false);
			Assert::AreEqual(reader.read_bit(), true);
			Assert::AreEqual(reader.read_bit(), false);
			Assert::AreEqual(reader.read_bit(), false);
			Assert::AreEqual(reader.read_bit(), true);
			Assert::AreEqual(reader.read_bit(), true);
			Assert::AreEqual(reader.read_bit(), false);
			Assert::AreEqual(reader.read_bit(), true);
		}

		TEST_METHOD(BitReader_ReadBytesOffset)
		{
			uint8_t data[] = { 0b00010101, 0b11100000, 0xFF };
			lib_pac::bit_reader reader(data);

			reader.seek(0, 3);
			Assert::AreEqual(reader.read_byte(), (uint8_t) 0b10101111);
			reader.seek(2, 0);
			Assert::AreEqual(reader.read_byte(), (uint8_t) 0xFF);
		}
	};
}