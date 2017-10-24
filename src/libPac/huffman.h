#pragma once
#include "defines.h"

#include <stdint.h>
#include <vector>

#include "bitstream.h"

namespace lib_pac
{
	struct bit_lookup
	{
		struct entry
		{
			uint32_t weight;
			uint32_t pattern;
			uint8_t lenght;
		};

		entry entries[0x100];

	public:
		bit_lookup();
	};

	class huffman_tree
	{
	private:
		struct node
		{
			node* left;
			node* right;
			uint32_t weight;
			uint8_t value;
			bool is_leaf;

			node();
			explicit node(uint8_t value);
			~node();
		};
		typedef node* node_ptr;

		node_ptr m_root;
		
	public:
		class node_cursor
		{
		private:
			friend class huffman_tree;
			node_ptr m_node;
			node_cursor() = delete;
			explicit node_cursor(node_ptr node);

		public:
			EXPORTS bool is_leaf() const;
			EXPORTS uint8_t get_value() const;
			EXPORTS void move_left();
			EXPORTS void move_right();
			EXPORTS void increase_weight() const;
		};

	private:
		static void bit_dump(bit_writer& buffer, const node& node);
		static void bit_read(bit_reader& buffer, node_ptr* node);
		static void generate_lookup(bit_lookup& lookup, const node& node, uint32_t bit_path, uint32_t bit_lenght);
		static void count(const node& node, size_t& n_branches, size_t& n_leaves);
		static void measure(const node& node, size_t& tree_bits, size_t& data_bits, size_t bit_lenght);
		static void recalculate_weight(node& node);
		static void reset_weight(node& node);

	public:
		EXPORTS void create(const uint8_t* data, size_t size);
		EXPORTS void read(bit_reader& reader);
		EXPORTS void write(bit_writer& buffer) const;
		EXPORTS void generate_lookup(bit_lookup& lookup) const;

		EXPORTS void recalculate_weights() const;
		EXPORTS void reset_weights() const;
		EXPORTS size_t node_count(size_t& branches, size_t& leaves) const;
		EXPORTS size_t bit_count(size_t& tree_bits, size_t& data_bits) const;
		EXPORTS size_t bit_count() const;

		EXPORTS node_cursor get_cursor() const;

		EXPORTS huffman_tree();
		EXPORTS ~huffman_tree();
	};
}
