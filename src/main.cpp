#include <iostream>
#include <sstream>
#include <fstream>

#include "huffman/huffman.hpp"

int main(int argc, const char* argv[]) {
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

	std::cout << '\n';

	std::ofstream output;
	output.open("out.hff", std::ios::binary | std::ios::out);

	message.serialize(output);

	output.close();

	std::ifstream input_file;
	input_file.open("out.hff", std::ios::binary | std::ios::in);

	auto deserialized_message = Huffman::EncodedMessage::deserialize(input_file);

	input_file.close();

	std::cout << "Encoded message: ";

	for(auto it = deserialized_message.message_buffer.bit_begin(); it != deserialized_message.message_buffer.bit_end(); ++it) {
		std::cout << (*it ? '1' : '0');
	}

	std::cout << '\n';

	std::cout << "Decoded message: ";
	Huffman::decode(deserialized_message, std::cout);
	std::cout << '\n';

	return 0;
}