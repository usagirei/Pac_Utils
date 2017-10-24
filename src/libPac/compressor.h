#pragma once
#include "defines.h"

#include <stdint.h>
#include "huffman.h"
#include <memory>

namespace lib_pac
{
	class compressor_info
	{
		friend class compressor;
	private:
		uint32_t m_input_size;
		uint32_t m_output_size;
		uint32_t m_block_size;
		uint32_t m_block_count;

		std::vector<std::unique_ptr<huffman_tree>> m_trees;
		std::vector<size_t> m_chunk_dec_sizes;
		std::vector<size_t> m_chunk_cmp_sizes;
		std::vector<size_t> m_chunk_data_offsets;
		const uint8_t* m_data_input;

		void set_input(const void* buf, uint32_t size);
		void set_output(uint32_t size);
		void set_blocks(uint32_t count, uint32_t maximum_size);
		bool set_data_chunk(int n, uint32_t decompressed_size, uint32_t compressed_size, uint32_t data_offset, std::unique_ptr<huffman_tree> tree);
		bool set_data_chunk(int n, uint32_t decompressed_size, uint32_t compressed_size, uint32_t data_offset);

		inline huffman_tree& trees(int i) const;
		inline uint32_t chunk_compressed_size(int i) const;
		inline uint32_t chunk_decompressed_size(int i) const;
		inline uint32_t chunk_data_offset(int i) const;
		inline uint32_t block_size() const;
		inline uint32_t block_count() const;

	public:
		compressor_info() = default;
		EXPORTS inline uint32_t input_size() const;
		EXPORTS inline uint32_t output_size() const;

		EXPORTS inline const uint8_t* input() const;
	};


	class compressor
	{
	public:
		EXPORTS static std::unique_ptr<compressor_info> prepare_compression(const char* data, size_t size, uint32_t block_size, uint32_t n_threads = 0);
		EXPORTS static void compress(const compressor_info& info, char* dst, uint32_t n_threads = 0);

		EXPORTS static std::unique_ptr<compressor_info> prepare_decompression(const char* data, size_t size);
		EXPORTS static void decompress(const compressor_info& info, char* dst, uint32_t n_threads = 0);
	};
}
