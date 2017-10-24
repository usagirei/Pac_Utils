#include "stdafx.h"
#include "CppUnitTest.h"
#include <bitstream.h>

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace libPac_Test
{
	TEST_CLASS(BitWriterTests)
	{
	public:

		TEST_METHOD(BitWriter_WriteBytes)
		{
			uint8_t expected[] = {0xFF, 0x7F, 0x64};
			uint8_t actual[sizeof(expected)] = {0xFF,0x7F,0xCC};
			
			lib_pac::bit_writer writer(actual);
			for(int i = 0; i < sizeof(expected); ++i)
				writer.write_bits(expected[i], 8);

			for(int i = 0; i < sizeof(expected); ++i)
				Assert::AreEqual(actual[i], expected[i]);
		}

		TEST_METHOD(BitWriter_WriteBytePartial)
		{
			uint8_t expected[] = { 0b101011'11, 0b0110'0111, 0b10'000101 };
			uint8_t actual[sizeof(expected)] = { };

			lib_pac::bit_writer writer(actual);
			writer.write_bits(0x2B, 6);
			writer.write_bits(0x36, 6);
			writer.write_bits(0x1E, 6);
			writer.write_bits(0x05, 6);

			for (int i = 0; i < sizeof(expected); ++i)
				Assert::AreEqual(actual[i], expected[i]);
		}

		TEST_METHOD(BitWriter_WriteInts)
		{
			uint8_t expected[] = { 0xFF, 0x7F, 0x64, 0x32 };
			uint8_t actual[sizeof(expected)] = { 0xFF,0x7F,0xCC,0xAA };

			lib_pac::bit_writer writer(actual);
			writer.write_bits(0xFF7F6432, 32);

			for (int i = 0; i < sizeof(expected); ++i)
				Assert::AreEqual(actual[i], expected[i]);
		}

		TEST_METHOD(BitWriter_WriteIntPartial)
		{
			uint8_t expected[] = { 0xAC, 0xF0, 0x53, 0x0F };
			uint8_t actual[sizeof(expected)] = { 0xFF,0x7F,0xCC };

			lib_pac::bit_writer writer(actual);
			writer.write_bits(0x159E0, 17);
			writer.write_bits(0x530F, 15);

			for (int i = 0; i < sizeof(expected); ++i)
				Assert::AreEqual(actual[i], expected[i]);
		}


		TEST_METHOD(BitWriter_WriteBits)
		{
			uint8_t expected[] = { 0b10101100, 0b11110000 };
			uint8_t actual[sizeof(expected)] = {0xFF,0x00};

			lib_pac::bit_writer writer(actual);

			writer.write_bit(true);
			writer.write_bit(false);
			writer.write_bit(true);
			writer.write_bit(false);
			writer.write_bit(true);
			writer.write_bit(true);
			writer.write_bit(false);
			writer.write_bit(false);

			writer.write_bit(true);
			writer.write_bit(true);
			writer.write_bit(true);
			writer.write_bit(true);
			writer.write_bit(false);
			writer.write_bit(false);
			writer.write_bit(false);
			writer.write_bit(false);

			for (int i = 0; i < sizeof(actual); ++i)
				Assert::AreEqual(actual[i], expected[i]);
		}

		TEST_METHOD(BitWriter_WriteBytesOffset)
		{
			uint8_t expected[] = { 0b00001001, 0b10100000 };
			uint8_t actual[sizeof(expected)] = {0x0F,0xF0};

			lib_pac::bit_writer writer(actual);
			writer.seek(0, 4);
			writer.write_bits(0x9A, 8);

			for (int i = 0; i < sizeof(actual); ++i)
				Assert::AreEqual(actual[i], expected[i]);
		}
	};
}