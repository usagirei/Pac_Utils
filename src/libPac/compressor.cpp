#include "compressor.h"
#include <future>
#include "semaphore.h"

static std::tuple<size_t, std::unique_ptr<lib_pac::huffman_tree>>
block_analyze(const uint8_t* data, uint32_t size, lib_pac::semaphore& limiter)
{
	lib_pac::critical_section _(limiter);

	auto unique = std::make_unique<lib_pac::huffman_tree>();
	unique->create(data, size);

	size_t compSize = 1 + (unique->bit_count() - 1) / 8;

	return std::make_tuple(compSize, std::move(unique));
}

static void
block_compress(uint8_t* dst, uint32_t dst_size, const uint8_t* src, uint32_t src_size, const lib_pac::huffman_tree& tree,
               lib_pac::semaphore& limiter)
{
	lib_pac::critical_section _(limiter);

	lib_pac::bit_writer writer(dst);

	tree.write(writer);
	lib_pac::bit_lookup lookup;
	tree.generate_lookup(lookup);

	for (uint32_t i = 0; i < src_size; i++)
	{
		const uint8_t value = src[i];
		auto& entry = lookup.entries[value];
		writer.write_bits(entry.pattern, entry.lenght);
	}
}

static void
block_decompress(uint8_t* dst, uint32_t dst_size, const uint8_t* src, uint32_t src_size, lib_pac::semaphore& limiter)
{
	lib_pac::critical_section _(limiter);

	lib_pac::bit_reader reader(src);

	lib_pac::huffman_tree tree;
	tree.read(reader);

	for(uint32_t i = 0; i < dst_size; ++i)
	{
		auto cur = tree.get_cursor();
		while(!cur.is_leaf())
		{
			const bool move_right = reader.read_bit();
			if (move_right)
				cur.move_right();
			else
				cur.move_left();
		}
		dst[i] = cur.get_value();
	}
}

// Compression

std::unique_ptr<lib_pac::compressor_info>
lib_pac::compressor::prepare_compression(const char* input, size_t input_size, uint32_t block_size, uint32_t n_threads)
{
	if (!n_threads)
		n_threads = std::thread::hardware_concurrency();
	semaphore limiter(n_threads);

	int num_blocks = ((input_size - 1) / block_size) + 1;
	const uint8_t* data8 = reinterpret_cast<const uint8_t*>(input);

	auto info = std::make_unique<compressor_info>();

	info->set_input(data8, input_size);
	info->set_blocks(num_blocks, block_size);

	std::vector<std::future<std::tuple<size_t, std::unique_ptr<lib_pac::huffman_tree>>>> futures(num_blocks);

	for (int i = 0; i < num_blocks; ++i)
	{
		const uint32_t src_off = i * block_size;
		const uint32_t src_sz = std::min(block_size, input_size - src_off);
		futures[i] = std::async(std::launch::async, block_analyze, data8 + src_off, src_sz, std::ref(limiter));
	}

	uint32_t output_size = 16 + 12 * num_blocks;
	uint32_t chunk_offset = 0;
	for (int i = 0; i < num_blocks; ++i)
	{
		const uint32_t src_off = i * block_size;
		const uint32_t src_sz = std::min(block_size, input_size - src_off);

		auto tuple = futures[i].get();
		const size_t cmp_size = std::get<0>(tuple);

		info->set_data_chunk(i, src_sz, cmp_size, chunk_offset, std::move(std::get<1>(tuple)));
		output_size += cmp_size;
		chunk_offset += cmp_size;
	}
	info->set_output(output_size);

	return std::move(info);
}

void
lib_pac::compressor::compress(const compressor_info& info, char* dst, uint32_t n_threads)
{
	if (!n_threads)
		n_threads = std::thread::hardware_concurrency();

	const uint32_t blk_cnt = info.block_count();
	const uint32_t blk_sz = info.block_size();
	const uint32_t input_sz = info.input_size();
	const uint8_t* input_buf = info.input();

	std::vector<std::future<void>> futures(blk_cnt);

	semaphore limiter(n_threads);
	uint8_t* dst8 = reinterpret_cast<uint8_t*>(dst);
	size_t headerSize = 16 + 12 * blk_cnt;

	uint32_t* dst32 = reinterpret_cast<uint32_t*>(dst);
	*dst32++ = 0x1234;
	*dst32++ = blk_cnt;
	*dst32++ = blk_sz;
	*dst32++ = headerSize;

	uint32_t src_offset = 0;
	for (int i = 0; i < blk_cnt; ++i)
	{
		const uint32_t out_chunk_sz = info.chunk_compressed_size(i);
		const uint32_t in_chunk_sz = info.chunk_decompressed_size(i);
		const uint32_t dst_offset = info.chunk_data_offset(i);

		*dst32++ = in_chunk_sz;
		*dst32++ = out_chunk_sz;
		*dst32++ = dst_offset;

		huffman_tree& tree = info.trees(i);
		futures[i] = std::async(std::launch::async,
		                        block_compress,
		                        dst8 + headerSize + dst_offset,
								out_chunk_sz,
		                        input_buf + src_offset,
		                        in_chunk_sz,
		                        std::ref(tree),
		                        std::ref(limiter));

		src_offset += in_chunk_sz;
	}

	for (int i = 0; i < blk_cnt; ++i)
		futures[i].get();
}

// Decompression

std::unique_ptr<lib_pac::compressor_info>
lib_pac::compressor::prepare_decompression(const char* data, size_t size)
{
	const uint32_t* data32 = reinterpret_cast<const uint32_t*>(data);

	const uint32_t magic = *data32++;
	if (magic != 0x1234)
		return std::unique_ptr<compressor_info>(nullptr);

	const uint32_t blk_cnt = *data32++;
	const uint32_t blk_sz = *data32++;
	const uint32_t header_sz = *data32++;

	if (header_sz != 16 + 12 * blk_cnt)
		return std::unique_ptr<compressor_info>(nullptr);

	auto info = std::make_unique<compressor_info>();
	info->set_input(data, size);
	info->set_blocks(blk_cnt, blk_sz);

	uint32_t output_size = 0;
	for (uint32_t i = 0; i < blk_cnt; ++i)
	{
		const uint32_t dec_sz = *data32++;
		const uint32_t cmp_sz = *data32++;
		const uint32_t offset = *data32++;

		output_size += dec_sz;
		info->set_data_chunk(i, dec_sz, cmp_sz, offset);
	}
	info->set_output(output_size);

	return std::move(info);
}

void
lib_pac::compressor::decompress(const compressor_info& info, char* dst, uint32_t n_threads)
{
	if (!n_threads)
		n_threads = std::thread::hardware_concurrency();

	const uint32_t blk_cnt = info.block_count();
	const uint32_t blk_sz = info.block_size();
	const uint32_t input_sz = info.input_size();
	const uint8_t* input_buf = info.input();

	std::vector<std::future<void>> futures(blk_cnt);

	semaphore limiter(n_threads);
	uint8_t* dst8 = reinterpret_cast<uint8_t*>(dst);
	size_t headerSize = 16 + 12 * blk_cnt;

	uint32_t dst_offset = 0;
	for (int i = 0; i < blk_cnt; ++i)
	{
		const uint32_t in_chunk_sz = info.chunk_compressed_size(i);
		const uint32_t out_chunk_sz = info.chunk_decompressed_size(i);
		const uint32_t src_offset = info.chunk_data_offset(i);

		huffman_tree& tree = info.trees(i);
		futures[i] = std::async(std::launch::async,
			block_decompress,
			dst8 + dst_offset,
			out_chunk_sz,
			input_buf + src_offset + headerSize,
			in_chunk_sz,
			std::ref(limiter));

		dst_offset += out_chunk_sz;
	}

	for (int i = 0; i < blk_cnt; ++i)
		futures[i].get();
}

// Compression Info Getters

void
lib_pac::compressor_info::set_input(const void* buf, uint32_t size)
{
	m_input_size = size;
	m_data_input = static_cast<const uint8_t*>(buf);
}

void
lib_pac::compressor_info::set_output(uint32_t size)
{
	m_output_size = size;
}

void
lib_pac::compressor_info::set_blocks(uint32_t count, uint32_t size)
{
	m_block_count = count;
	m_block_size = size;

	m_trees.resize(count);
	m_chunk_cmp_sizes.resize(count);
	m_chunk_dec_sizes.resize(count);
	m_chunk_data_offsets.resize(count);
}

bool
lib_pac::compressor_info::set_data_chunk(int n, uint32_t decompressed_size, uint32_t compressed_size,
                                         uint32_t data_offset, std::unique_ptr<huffman_tree> tree)
{
	if (n > m_block_count)
		return false;

	m_trees[n] = std::move(tree);
	m_chunk_cmp_sizes[n] = compressed_size;
	m_chunk_dec_sizes[n] = decompressed_size;
	m_chunk_data_offsets[n] = data_offset;
	return true;
}

bool
lib_pac::compressor_info::set_data_chunk(int n, uint32_t decompressed_size, uint32_t compressed_size,
                                         uint32_t data_offset)
{
	return set_data_chunk(n, decompressed_size, compressed_size, data_offset, std::unique_ptr<huffman_tree>(nullptr));
}


uint32_t
lib_pac::compressor_info::input_size() const
{
	return m_input_size;
}

uint32_t
lib_pac::compressor_info::output_size() const
{
	return m_output_size;
}

uint32_t
lib_pac::compressor_info::block_size() const
{
	return m_block_size;
}

uint32_t
lib_pac::compressor_info::block_count() const
{
	return m_block_count;
}

lib_pac::huffman_tree&
lib_pac::compressor_info::trees(int i) const
{
	return *m_trees[i];
}

uint32_t
lib_pac::compressor_info::chunk_compressed_size(int i) const
{
	return m_chunk_cmp_sizes[i];
}

uint32_t
lib_pac::compressor_info::chunk_decompressed_size(int i) const
{
	return m_chunk_dec_sizes[i];
}

uint32_t
lib_pac::compressor_info::chunk_data_offset(int i) const
{
	return m_chunk_data_offsets[i];
}

const uint8_t*
lib_pac::compressor_info::input() const
{
	return m_data_input;
}
