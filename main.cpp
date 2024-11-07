#include <iostream>
#include <sstream>

#include "huffman/huffman.hpp"

int main() {
	std::cout << "hello\n";

	std::string input;

	std::cin >> input;

	std::stringstream input_stream(input);

	Huffman::encode(input_stream);

	return 0;
}