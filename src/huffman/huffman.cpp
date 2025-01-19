#include "huffman.hpp"

#include "tree/tree.hpp"

#include <algorithm>
#include <array>
#include <iterator>
#include <utility>
#include <queue>
#include <vector>

Huffman::EncodedMessage Huffman::encode(std::istream& input) {
	// Count occurances (probablities) of the characters in the text
	std::array<uint64_t, 255> occurances;
	std::fill(occurances.begin(), occurances.end(), 0);

	std::string message = "";

	input >> std::noskipws;

	char current_character, prev_character = '\0';
	bool multiple_chars = false;
	while(input >> current_character) {
		occurances[current_character - 1]++;
		message += current_character;

		if(prev_character != '\0' && prev_character != current_character) {
			multiple_chars = true;
		}

		prev_character = current_character;
	}

	if(!multiple_chars) {
		// Unfortunately we have to do that because of how we store the data
		// If we stored the length of the original message instead of the message buffer
		// We would be able to reconstruct the message, but that would require a rework
		// Of the `EncodedMessage` class
		throw OneCharacterSourceException();
	}

	// Create a list of one-node Huffman trees to store the characters with their probablities (occurence count)
	// Using a priority queue to mitigate the cost of looking for the least probable character in each iteration
	auto tree_priority_cmp = [](const Tree& left, const Tree& right) {
		return left.get_occurances() > right.get_occurances();
	};

	std::priority_queue<Tree, std::vector<Tree>, decltype(tree_priority_cmp)> tree_queue(tree_priority_cmp);

	for(uint16_t i = 0; i < occurances.size(); i++) {
		if(occurances[i] > 0) {
			tree_queue.emplace(i + 1, occurances[i]);
		}
	}

	// Fuse the trees into one Huffman tree
	while(tree_queue.size() > 1) {
		// We have to do it this way because of reasons explained below
		// https://stackoverflow.com/questions/20149471/move-out-element-of-std-priority-queue-in-c11
		Tree tree1 = std::move(const_cast<Tree&>(tree_queue.top()));
		tree_queue.pop();

		Tree tree2 = std::move(const_cast<Tree&>(tree_queue.top()));
		tree_queue.pop();

		Tree new_tree(std::move(tree1), std::move(tree2));
		tree_queue.push(std::move(new_tree));
	}

	Tree huffman_tree = std::move(const_cast<Tree&>(tree_queue.top()));

	// Get the code as a dictionary
	Huffman::Tree::CodeDictionary code_dictionary = huffman_tree.get_codes();

	// Encode the message into a bit buffer
	Buffer buffer;

	for(char current : message) {
		auto code = code_dictionary[current];

		buffer <<= code;
	}

	EncodedMessage result = { std::move(huffman_tree), std::move(buffer) };

	return result;
}

void Huffman::decode(const EncodedMessage& input, std::ostream& output) {
	// Get the code as a dictionary
	Huffman::Tree::CharacterDictionary code_dictionary = input.huffman_tree.get_codes_for_decoding();

	// Decode the message
	Buffer current;

	for(auto it = input.message_buffer.bit_begin(); it != input.message_buffer.bit_end(); ++it) {
		current <<= *it;

		if(code_dictionary.find(current) != code_dictionary.end()) {
			output << code_dictionary[current];
			current = Buffer();
		}
	}
}

const char* Huffman::OneCharacterSourceException::what() const noexcept {
	return "Encoding single character sequences is currently unsupported.";
}