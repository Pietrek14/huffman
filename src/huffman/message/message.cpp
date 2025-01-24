#include "message.hpp"

#include <string>
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

void Huffman::EncodedMessage::ensure_stream_read_success(std::istream& stream) {
	if(stream.eof()) {
		throw UnexpectedEofException();
	}
}

Huffman::EncodedMessage Huffman::EncodedMessage::deserialize(std::istream& input) {
	// Header section
	char header_section[4];
	input.read(header_section, 4);
	ensure_stream_read_success(input);

	std::string header = std::string() + header_section[0] + header_section[1] + header_section[2];

	if(header != "HFF") {
		throw InvalidHeaderException(header, "HFF");
	}

	if(header_section[3] != Huffman::CURRENT_VERSION) {
		throw WrongVersionException(header_section[3], Huffman::CURRENT_VERSION);
	}

	// Content section
	char tree_size_chars[2];
	input.read(tree_size_chars, 2);
	ensure_stream_read_success(input);
	std::byte tree_size_bytes[2];
	for(uint8_t i = 0; i < 4; i++) {
		tree_size_bytes[i] = std::byte(tree_size_chars[i]);
	}
	uint16_t tree_size = std::to_integer<uint16_t>(tree_size_bytes[0]);
	tree_size |= std::to_integer<uint16_t>(tree_size_bytes[1]) << 8;

	char message_size_chars[4];
	input.read(message_size_chars, 4);
	ensure_stream_read_success(input);
	std::byte message_size_bytes[4];
	for(uint8_t i = 0; i < 4; i++) {
		message_size_bytes[i] = std::byte(message_size_chars[i]);
	}
	uint32_t message_size = std::to_integer<uint32_t>(message_size_bytes[0]);
	message_size |= std::to_integer<uint32_t>(message_size_bytes[1]) << 8;
	message_size |= std::to_integer<uint32_t>(message_size_bytes[2]) << 16;
	message_size |= std::to_integer<uint32_t>(message_size_bytes[3]) << 24;

	uint64_t content_buffer_bits_num = tree_size + message_size;
	uint8_t padding_size = content_buffer_bits_num % 8;
	uint32_t content_buffer_bytes_num = content_buffer_bits_num / 8 + (padding_size > 0);

	// Read the content bytes in 64 byte buffers
	uint32_t bytes_left = content_buffer_bytes_num;
	const uint32_t temp_buffer_size = 64;

	Buffer tree_buffer, message_buffer;
	tree_buffer.reserve_bytes(tree_size / 8 + (tree_size % 8 > 0));
	message_buffer.reserve_bytes(message_size / 8 + (message_size % 8 > 0));

	bool on_message = false;
	uint64_t message_bits_read = 0;

	while(bytes_left > 0) {
		char content_bytes[temp_buffer_size];

		uint32_t bytes_read_this_iteration = bytes_left > temp_buffer_size ? temp_buffer_size : bytes_left;

		input.read(content_bytes, bytes_read_this_iteration);
		ensure_stream_read_success(input);

		bytes_left -= bytes_read_this_iteration;
		uint32_t bytes_read = content_buffer_bytes_num - bytes_left;

		if(bytes_read < tree_size / 8) {
			// Add the bytes to the tree buffer
			for(uint32_t i = 0; i < bytes_read_this_iteration; i++) {
				tree_buffer <<= std::byte(content_bytes[i]);
			}
		} else if(!on_message) {
			uint32_t byte_index = 0;

			// First, go over the bytes that contain only the tree information
			for(; byte_index < bytes_read_this_iteration - (bytes_read - tree_size / 8); byte_index++) {
				tree_buffer <<= std::byte(content_bytes[byte_index]);
			}

			// Divide the common byte between the two buffers
			uint8_t bit_index = 0;

			// The byte that contains both tree and message information
			std::byte common_byte = std::byte(content_bytes[byte_index]);

			for(; bit_index < tree_size % 8; bit_index++) {
				tree_buffer <<= static_cast<bool>((common_byte >> (7 - bit_index)) & std::byte(1));
			}

			for(; bit_index < 8 && message_size - message_bits_read > 0; bit_index++) {
				message_buffer <<= static_cast<bool>((common_byte >> (7 - bit_index)) & std::byte(1));
				message_bits_read++;
			}

			byte_index++;

			// Add the rest of the bytes to the message buffer
			for(; byte_index < bytes_read_this_iteration && message_size - message_bits_read >= 8; byte_index++) {
				message_buffer <<= std::byte(content_bytes[byte_index]);
				message_bits_read += 8;
			}

			// Add bits without padding
			if(message_bits_read != message_size && message_size - message_bits_read < 8) {
				for(uint8_t i = 0; i < message_size - message_bits_read; i++) {
					message_buffer <<= static_cast<bool>(content_bytes[bytes_read_this_iteration - 1] & (1 << (7 - i)));
				}
			}

			// On the next iterations, simply add the bytes to the message buffer
			on_message = true;
		} else {
			// Add the bytes to the message buffer
			for(uint32_t i = 0; i < bytes_read_this_iteration && message_size - message_bits_read >= 8; i++) {
				message_buffer <<= std::byte(content_bytes[i]);
				message_bits_read += 8;
			}

			// Add bits without padding
			if(message_bits_read != message_size && message_size - message_bits_read < 8) {
				for(uint8_t i = 0; i < message_size - message_bits_read; i++) {
					message_buffer <<= static_cast<bool>(content_bytes[bytes_read_this_iteration - 1] & (1 << (7 - i)));
				}
			}
		}
	}

	try {
		Tree tree = Tree::deserialize(tree_buffer);

		// Footer section
		char footer_bytes[2];
		input.read(footer_bytes, 2);
		ensure_stream_read_success(input);

		if(footer_bytes[0] != 'X' || footer_bytes[1] != 'X') {
			throw InvalidFooterException(std::string() + footer_bytes[0] + footer_bytes[1], "XX");
		}

		return { std::move(tree), message_buffer };
	} catch(const Tree::DeserializationException& e) {
		throw InvalidTreeDataException();
	}
}

Huffman::EncodedMessage::DeserializationException::DeserializationException() {}

const char* Huffman::EncodedMessage::DeserializationException::what() const noexcept {
	return m_Message.c_str();
}

Huffman::EncodedMessage::UnexpectedEofException::UnexpectedEofException() {
	m_Message = "The stream ended unexpectedly. Make sure you have all of the serialized data.";
}

Huffman::EncodedMessage::InvalidHeaderException::InvalidHeaderException(std::string file_header, std::string expected_header) {
	m_FileHeader = file_header;
	m_ExpectedHeader = expected_header;

	m_Message = "Invalid file header. Expected '" + expected_header + "', got '" + file_header + "'.";
}

std::string Huffman::EncodedMessage::InvalidHeaderException::get_file_header() const {
	return m_FileHeader;
}

std::string Huffman::EncodedMessage::InvalidHeaderException::get_expected_header() const {
	return m_ExpectedHeader;
}

Huffman::EncodedMessage::WrongVersionException::WrongVersionException(uint8_t file_version, uint8_t software_version) {
	m_FileVersion = file_version;
	m_SoftwareVersion = software_version;

	m_Message = "Your software is out of date. You're using ver " + std::to_string(static_cast<uint32_t>(file_version));
}

uint8_t Huffman::EncodedMessage::WrongVersionException::get_file_version() const {
	return m_FileVersion;
}

uint8_t Huffman::EncodedMessage::WrongVersionException::get_software_version() const {
	return m_SoftwareVersion;
}

Huffman::EncodedMessage::InvalidTreeDataException::InvalidTreeDataException() {
	m_Message = "Serialized tree data is invalid.";
}

Huffman::EncodedMessage::InvalidFooterException::InvalidFooterException(std::string file_footer, std::string expected_footer) {
	m_FileFooter = file_footer;
	m_ExpectedFooter = expected_footer;

	m_Message = "Invalid footer. Expected '" + expected_footer + "', got '" + file_footer + "'.";
}

std::string Huffman::EncodedMessage::InvalidFooterException::get_file_footer() const {
	return m_FileFooter;
}

std::string Huffman::EncodedMessage::InvalidFooterException::get_expected_footer() const {
	return m_ExpectedFooter;
}
