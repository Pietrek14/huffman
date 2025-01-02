#include <iostream>
#include <sstream>
#include <fstream>

#include "huffman/huffman.hpp"

int main() {
	std::cout << "hello\n";

	std::string input;

	std::cin >> input;

	std::stringstream input_stream(input);

	Huffman::EncodedMessage message = Huffman::encode(input_stream);

	std::cout << "Encoded message: ";

	auto buffer = message.message_buffer;

	for(auto it = buffer.bit_begin(); it != buffer.bit_end(); ++it) {
		std::cout << (*it ? '1' : '0');
	}

	std::ofstream output;
	output.open("out.hff", std::ios::binary | std::ios::out);

	message.serialize(output);

	output.close();

	return 0;
}