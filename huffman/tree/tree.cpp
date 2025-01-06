#include "tree.hpp"

#include <utility>

// Node methods definitions
Huffman::Tree::Node::Node(uint8_t character, uint64_t occurances)
	: m_Character(character), m_Occurences(occurances) {}

void Huffman::Tree::Node::push_left(std::unique_ptr<Huffman::Tree::Node>&& node) {
	m_Left = std::move(node);
}

void Huffman::Tree::Node::push_right(std::unique_ptr<Huffman::Tree::Node>&& node) {
	m_Right = std::move(node);
}

uint8_t Huffman::Tree::Node::get_character() const {
	return m_Character;
}

uint64_t Huffman::Tree::Node::get_occurances() const {
	return m_Occurences;
}

const std::unique_ptr<Huffman::Tree::Node>& Huffman::Tree::Node::get_left() const {
	return m_Left;
}

const std::unique_ptr<Huffman::Tree::Node>& Huffman::Tree::Node::get_right() const {
	return m_Right;
}

// Tree methods definitions
Huffman::Tree::Tree(uint8_t character, uint64_t occurences) {
	m_Root = std::make_unique<Node>(character, occurences);
}

Huffman::Tree::Tree(Tree&& left_tree, Tree&& right_tree) {
	// Using the null character for parent nodes
	m_Root = std::make_unique<Node>('\0', left_tree.get_occurances() + right_tree.get_occurances());
	m_Root->push_left(std::move(left_tree.m_Root));
	m_Root->push_right(std::move(right_tree.m_Root));
}

uint64_t Huffman::Tree::get_occurances() const {
	return m_Root->get_occurances();
}

void Huffman::Tree::generate_codes(CodeDictionary& code_dict, const std::unique_ptr<Node>& current_node, Code current_code) const {
	if(current_node->get_character() != '\0') {
		code_dict[current_node->get_character()] = current_code;
		return;
	}

	Code left_code = current_code;
	left_code.push_back(false);

	Code right_code = current_code;
	right_code.push_back(true);

	generate_codes(code_dict, current_node->get_left(), left_code);
	generate_codes(code_dict, current_node->get_right(), right_code);
}

Huffman::Tree::CodeDictionary Huffman::Tree::get_codes() const {
	CodeDictionary result;

	generate_codes(result, m_Root, Code());

	return result;
}

Huffman::Buffer Huffman::Tree::serialize() const {
	Buffer output;

	preorder_serialization(output, m_Root);

	return output;
}

void Huffman::Tree::preorder_serialization(Buffer& output, const std::unique_ptr<Node>& current_node) const {
	if(current_node->get_character()) {
		output <<= true;

		output <<= std::byte(current_node->get_character());
	} else {
		output <<= false;

		preorder_serialization(output, current_node->get_left());
		preorder_serialization(output, current_node->get_right());
	}
}

Huffman::Tree Huffman::Tree::deserialize(const Buffer& buffer) {
	auto it = buffer.bit_begin();

	if(*it) {
		++it;
		return Tree(std::to_integer<uint8_t>(it.next_byte()), 0);
	}

	++it;

	Tree result('\0', 0);

	preorder_deserialization(result.m_Root, it);

	return result;
}

void Huffman::Tree::preorder_deserialization(std::unique_ptr<Node>& root, Buffer::BitIterator& input) {
	for(int i = 0; i < 2; i++) {
		bool character_in_node = *input;
		++input;

		if(character_in_node) {
			auto character = std::to_integer<uint8_t>(input.next_byte());

			root->push_left(std::make_unique<Node>(character, 0));
		} else {
			auto node = std::make_unique<Node>('\0', 0);
			preorder_deserialization(node, input);
			if(i == 0)
				root->push_left(std::move(node));
			else
				root->push_right(std::move(node));
		}
	}
}
