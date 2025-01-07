#include "message.hpp"

#include <vector>

#include "../info.hpp"

void Huffman::EncodedMessage::serialize(std::ostream& output) const {
	// Header section
	output.write("HFF", 3);
	uint8_t version_arr[1] = { Huffman::CURRENT_VERSION };
	char* version_bytes = reinterpret_cast<char*>(version_arr);
	output.write(version_bytes, 1);

	// Content section
	Buffer tree_buffer = huffman_tree.serialize();

	Buffer content_buffer = tree_buffer;

	content_buffer <<= message_buffer;

	// This has to be done without reinterpret cast to not assume endianness
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

// TODO: Implement exceptions for this function
#include <iostream>

Huffman::EncodedMessage Huffman::EncodedMessage::deserialize(std::istream& input) {
	// Header section
	char header_section[4];
	input.read(header_section, 4);

	if(header_section[0] != 'H' || header_section[1] != 'F' || header_section[2] != 'F') {
		std::cerr << "Invalid header! Given file may not be a hff file.\n";
		throw 1;
	}

	if(header_section[3] != Huffman::CURRENT_VERSION) {
		std::cerr << "Your software is out of date. File was encoded with ver " << (int)header_section[3]
			<< ". You're using ver " << (int)Huffman::CURRENT_VERSION << '\n';
		throw 1;
	}

	// Content section

	// TODO: Check if the bytes are actually in the file
	char tree_size_bytes[2];
	input.read(tree_size_bytes, 2);
	uint16_t tree_size = tree_size_bytes[0];
	tree_size |= tree_size_bytes[1] << 8;

	char message_size_bytes[4];
	input.read(message_size_bytes, 4);
	uint32_t message_size = message_size_bytes[0];
	message_size |= message_size_bytes[1] << 8;
	message_size |= message_size_bytes[2] << 16;
	message_size |= message_size_bytes[3] << 24;

	uint64_t content_buffer_bits_num = tree_size + message_size;
	uint16_t content_buffer_bytes_num = content_buffer_bits_num / 8 + (content_buffer_bits_num % 8 > 0);

	// This can fail if content_buffer_bytes_num is too large
	char* content_buffer_bytes = new char[content_buffer_bytes_num];

	// This can fail if the file ends unexpectedly
	input.read(content_buffer_bytes, content_buffer_bytes_num);

	Buffer content_buffer;

	for(uint16_t i = 0; i < content_buffer_bytes_num - 1; i++) {
		content_buffer <<= std::byte(content_buffer_bytes[i]);
	}

	for(uint8_t i = 0; i < content_buffer_bits_num % 8; i++) {
		content_buffer <<= static_cast<bool>(content_buffer_bytes[content_buffer_bytes_num - 1] & (1 << (7 - i)));
	}

	// If there's no padding
	if(content_buffer_bits_num % 8 == 0) {
		content_buffer <<= std::byte(content_buffer_bytes[content_buffer_bytes_num - 1]);
	}

	delete[] content_buffer_bytes;

	// TODO: There are quicker ways to do this
	Buffer tree_buffer, message_buffer;
	uint64_t index = 0;

	for(auto it = content_buffer.bit_begin(); it != content_buffer.bit_end(); ++it) {
		if(index < tree_size) {
			tree_buffer <<= *it;
		} else {
			message_buffer <<= *it;
		}

		index++;
	}
	Tree tree = Tree::deserialize(tree_buffer);

	// Footer section
	char footer_bytes[2];

	input.read(footer_bytes, 2);

	if(footer_bytes[0] != 'X' || footer_bytes[1] != 'X') {
		std::cerr << "Invalid footer! Expected 'XX', got '" << footer_bytes[0] << footer_bytes[1] << '\n';
		throw 1;
	}

	return { std::move(tree), message_buffer };
}