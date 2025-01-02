#include "message.hpp"

#include "../info.hpp"

void Huffman::EncodedMessage::serialize(std::ostream& output) const {
	// Header section
	output.write("HFF", 3);
	uint8_t version_arr[1] = { Huffman::CURRENT_VERSION };
	char* version_bytes = reinterpret_cast<char*>(version_arr);
	output.write(version_bytes, 1);

	// Content section
	Buffer content_buffer = message_buffer;
	Buffer tree_buffer = huffman_tree.serialize();

	content_buffer <<= tree_buffer;

	// This has to be done without reinterpret cast to not presume endianness
	uint16_t tree_size = tree_buffer.get_length();
	char tree_size_bytes[2] = {
		static_cast<char>(tree_size),
		static_cast<char>(tree_size >> 8),
	};

	output.write(tree_size_bytes, 2);

	uint32_t message_size = message_buffer.get_length();
	char message_size_bytes[4] = {
		static_cast<char>(message_size),
		static_cast<char>(message_size >> 8),
		static_cast<char>(message_size >> 16),
		static_cast<char>(message_size >> 24),
	};

	output.write(message_size_bytes, 4);

	for(std::byte byte : content_buffer) {
		char byte_char[1] = { std::to_integer<char>(byte) };
		output.write(byte_char, 1);
	}

	// Footer section
	output.write("XX", 2);
}