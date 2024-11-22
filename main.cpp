#include <iostream>
#include <sstream>

#include "huffman/huffman.hpp"

int main() {
	std::cout << "hello\n";

	std::string input;

	std::cin >> input;

	std::stringstream input_stream(input);

	Huffman::EncodedMessage message = Huffman::encode(input_stream);

	std::cout << "Encoded message: ";

	auto buffer = message.messageBuffer;

	for(auto it = buffer.bit_begin(); it != buffer.bit_end(); ++it) {
		std::cout << (*it ? '1' : '0');
	}

	return 0;
}