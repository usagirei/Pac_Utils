#include "huffman.h"
#include <algorithm>
#include <array>
#include <vector>
#include <queue>
#include <list>
#include <bitset>


void
lib_pac::huffman_tree::create(const uint8_t* data, size_t size)
{
	if (m_root != nullptr)
		delete m_root;

	std::array<node_ptr, 256> node_array{};
	int node_count = 0;
	for (size_t i = 0; i < size; ++i)
	{
		const uint8_t value = data[i];
		if (!node_array[value])
		{
			node_array[value] = new node(value);
			++node_count;
		}
		node_array[value]->weight++;
	}

	std::list<node_ptr> node_list(node_count);
	const auto node_not_null = [](const node_ptr& a) -> bool
	{
		return a != nullptr;
	};
	std::copy_if(node_array.begin(), node_array.end(), node_list.begin(), node_not_null);

	const auto node_comparer = [](const node_ptr& a, const node_ptr& b) -> bool
	{
		return a->weight < b->weight;
	};
	node_list.sort(node_comparer);

	while (node_list.size() > 1)
	{
		// Pop Two Nodes
		node_ptr a = node_list.front();
		node_list.erase(node_list.begin());
		node_ptr b = node_list.front();
		node_list.erase(node_list.begin());

		// Create a Third
		node_ptr c = new node();
		c->left = a;
		c->right = b;
		c->weight = a->weight + b->weight;

		// Insert it sorted
		auto it = node_list.begin();
		while (it != node_list.end() && (*it)->weight <= c->weight)
			std::advance(it, 1);
		node_list.insert(it, c);
	}

	m_root = node_list.front();
}

void
lib_pac::huffman_tree::read(bit_reader& reader)
{
	if (m_root != nullptr)
		delete m_root;

	bit_read(reader, &m_root);
}

void
lib_pac::huffman_tree::bit_read(bit_reader& buffer, node_ptr* n)
{
	const bool branch = buffer.read_bit();
	if (branch)
	{
		node* nn = new node;
		*n = nn;
		bit_read(buffer, &nn->left);
		bit_read(buffer, &nn->right);
	}
	else
	{
		const uint8_t value = buffer.read_byte();
		*n = new node(value);
	}
}

void
lib_pac::huffman_tree::bit_dump(bit_writer& buffer, const node& node)
{
	if (node.is_leaf) // Leaf
	{
		buffer.write_bit(false);
		buffer.write_bits(node.value, 8);
	}
	else // Branch
	{
		buffer.write_bit(true);
		bit_dump(buffer, *node.left);
		bit_dump(buffer, *node.right);
	}
}

void lib_pac::huffman_tree::generate_lookup(bit_lookup& lookup, const node& node, uint32_t bit_path,
                                            uint32_t bit_lenght)
{
	if (node.is_leaf) // Leaf
	{
		lookup.entries[node.value].pattern = bit_path;
		lookup.entries[node.value].lenght = bit_lenght;
		lookup.entries[node.value].weight = node.weight;
	}
	else // Branch
	{
		generate_lookup(lookup, *node.left, bit_path << 1 | 0, bit_lenght + 1);
		generate_lookup(lookup, *node.right, bit_path << 1 | 1, bit_lenght + 1);
	}
}

void
lib_pac::huffman_tree::count(const node& node, size_t& n_branches, size_t& n_leaves)
{
	if (node.is_leaf)
		n_leaves++;
	else
		n_branches++;

	if (node.left)
		count(*node.left, n_branches, n_leaves);
	if (node.right)
		count(*node.right, n_branches, n_leaves);
}

void
lib_pac::huffman_tree::measure(const node& node, size_t& tree_bits, size_t& data_bits, size_t bit_lenght)
{
	if (node.is_leaf)
	{
		tree_bits += 9;
		data_bits += bit_lenght * node.weight;
	}
	else
	{
		tree_bits += 1;
	}

	if (node.left)
		measure(*node.left, tree_bits, data_bits, bit_lenght + 1);
	if (node.right)
		measure(*node.right, tree_bits, data_bits, bit_lenght + 1);
}

void
lib_pac::huffman_tree::recalculate_weight(node& node)
{
	if (node.is_leaf)
		return;
	node.weight = 0;
	if (node.left)
	{
		recalculate_weight(*node.left);
		node.weight += node.left->weight;
	}
	if (node.right)
	{
		recalculate_weight(*node.right);
		node.weight += node.right->weight;
	}
}

void
lib_pac::huffman_tree::reset_weight(node& node)
{
	node.weight = 0;
	if (node.left)
		reset_weight(*node.left);
	if (node.right)
		reset_weight(*node.right);
}


void
lib_pac::huffman_tree::write(lib_pac::bit_writer& buffer) const
{
	bit_dump(buffer, *m_root);
}

void
lib_pac::huffman_tree::recalculate_weights() const
{
	if (m_root)
		recalculate_weight(*m_root);
}

void
lib_pac::huffman_tree::reset_weights() const
{
	if (m_root)
		reset_weight(*m_root);
}

void
lib_pac::huffman_tree::generate_lookup(bit_lookup& lookup) const
{
	generate_lookup(lookup, *m_root, 0, 0);
}

size_t
lib_pac::huffman_tree::node_count(size_t& branches, size_t& leaves) const
{
	count(*m_root, branches, leaves);
	return branches + leaves;
}

size_t
lib_pac::huffman_tree::bit_count(size_t& tree_bits, size_t& data_bits) const
{
	tree_bits = data_bits = 0;
	measure(*m_root, tree_bits, data_bits, 0);
	return tree_bits + data_bits;
}

size_t
lib_pac::huffman_tree::bit_count() const
{
	size_t tree_bits = 0, data_bits = 0;
	measure(*m_root, tree_bits, data_bits, 0);
	return tree_bits + data_bits;
}

lib_pac::huffman_tree::node_cursor
lib_pac::huffman_tree::get_cursor() const
{
	node_cursor cur(m_root);
	return cur;
}

lib_pac::huffman_tree::huffman_tree() : m_root(nullptr)
{
}

lib_pac::huffman_tree::~huffman_tree()
{
	if (m_root)
		delete m_root;
}

lib_pac::bit_lookup::bit_lookup() : entries{}
{
}

lib_pac::huffman_tree::node::node() : left(nullptr), right(nullptr), weight(0), value(0), is_leaf(false)
{
}

lib_pac::huffman_tree::node::node(uint8_t value) : left(nullptr), right(nullptr), weight(0), value(value), is_leaf(true)
{
}

lib_pac::huffman_tree::node::~node()
{
	if (left)
		delete left;
	if (right)
		delete right;
}

lib_pac::huffman_tree::node_cursor::node_cursor(node_ptr node) : m_node(node)
{
}

bool
lib_pac::huffman_tree::node_cursor::is_leaf() const
{
	return m_node->is_leaf;
}

uint8_t
lib_pac::huffman_tree::node_cursor::get_value() const
{
	return m_node->value;
}

void
lib_pac::huffman_tree::node_cursor::move_left()
{
	m_node = m_node->left;
}

void
lib_pac::huffman_tree::node_cursor::move_right()
{
	m_node = m_node->right;
}

void
lib_pac::huffman_tree::node_cursor::increase_weight() const
{
	m_node->weight++;
}
