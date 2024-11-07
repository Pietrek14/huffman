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
