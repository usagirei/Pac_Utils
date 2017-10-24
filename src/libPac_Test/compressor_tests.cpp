#include "stdafx.h"
#include "CppUnitTest.h"
#include <compressor.h>
#include <random>
#include <array>
#include <functional>
#include <fstream>

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace libPac_Test
{
	TEST_CLASS(CompressorTests)
	{
	public:

		TEST_METHOD(Compressor_Cycle)
		{
			std::ifstream infile(__FILE__, std::ios::binary);
			infile.seekg(0, std::ios_base::end);
			int inSize = infile.tellg();
			infile.seekg(0, std::ios_base::beg);
			auto* input = new std::vector<char>(inSize * 10);
			infile.read(input->data(), input->size());

			for (int i = 1; i < 10; i++)
				std::memcpy(input->data() + i * inSize, input->data(), inSize);

			std::random_device rnd_device;
			// Specify the engine and distribution.
			std::mt19937 eng(rnd_device());
			std::normal_distribution<float> dist(127, 5);

			auto cinfo = lib_pac::compressor::prepare_compression(input->data(), input->size(), 0x2000, 0);
			auto* comp = new std::vector<char>(cinfo->output_size());
			lib_pac::compressor::compress(*cinfo, comp->data(), cinfo->output_size());

			auto dinfo = lib_pac::compressor::prepare_decompression(comp->data(), comp->size());
			auto* dec = new std::vector<char>(dinfo->output_size());
			lib_pac::compressor::decompress(*dinfo, dec->data(), dec->size());

			Assert::AreEqual(input->size(), dec->size());
			int rv = std::memcmp(input->data(), dec->data(), dec->size());
			Assert::AreEqual(rv, 0);

			delete input;
			delete comp;
			delete dec;
		}
	};
}