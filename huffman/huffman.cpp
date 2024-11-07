#include "huffman.hpp"

#include "tree/tree.hpp"

#include <algorithm>
#include <array>
#include <iterator>
#include <utility>
#include <queue>
#include <vector>

#include <iostream>

std::ostream Huffman::encode(std::istream& input) {
	// Count occurances (probablities) of the characters in the text
	std::array<uint64_t, 255> occurances;
	std::fill(occurances.begin(), occurances.end(), 0);

	uint8_t current_character;
	while(input >> current_character) {
		occurances[current_character - 1]++;
	}
	input.seekg(0);

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
		// TODO: If there's time left, implement own priority queue to avoid this dumb shit
		// https://stackoverflow.com/questions/20149471/move-out-element-of-std-priority-queue-in-c11
		Tree tree1 = std::move(const_cast<Tree&>(tree_queue.top()));
		tree_queue.pop();

		Tree tree2 = std::move(const_cast<Tree&>(tree_queue.top()));
		tree_queue.pop();

		// Because we're using a priority queue, we don't have to check which tree has the bigger probability
		Tree new_tree(std::move(tree1), std::move(tree2));
		tree_queue.push(std::move(new_tree));
	}

	Tree huffman_tree = std::move(const_cast<Tree&>(tree_queue.top()));

	// Get the code as a dictionary
	Huffman::Tree::CodeDictionary code_dictionary = huffman_tree.get_codes();

	for(auto [character, code] : code_dictionary) {
		std::cout << character << ": ";

		for(bool bit : code) {
			std::cout << (bit ? '1' : '0');
		}

		std::cout << std::endl;
	}
}