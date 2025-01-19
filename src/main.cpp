#include <iostream>
#include <vector>
#include <stdexcept>

#include "huffman/buffer/buffer.hpp"
#include "interface.hpp"

int main(int argc, const char* argv[]) {
	// Huffman::Buffer buffer1, buffer2;

	// buffer1 <<= std::byte('a');

	// for(auto it = buffer1.bit_begin(); it != buffer1.bit_end(); ++it) {
	// 	std::cout << *it;
	// }

	// std::cout << '\n';

	// buffer2 <<= buffer1;

	// for(auto byte : buffer2) {
	// 	std::cout << (char)byte;
	// }
	// for(auto it = buffer2.bit_begin(); it != buffer2.bit_end(); ++it) {
	// 	std::cout << *it;
	// }

	// return 0;

	std::vector<std::string> args(argc);

	for(uint16_t i = 0; i < argc; i++) {
		args[i] = argv[i];
	}

	try {
		const Action action(args);

		action.perform();
	} catch(const std::exception& e) {
		std::cerr << e.what();
		return 1;
	}

	return 0;
}