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

	// TODO: Check if the bytes are actually in the file
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
	uint16_t content_buffer_bytes_num = content_buffer_bits_num / 8 + (content_buffer_bits_num % 8 > 0);

	// This can fail if content_buffer_bytes_num is too large
	char* content_buffer_bytes = new char[content_buffer_bytes_num];

	// This can fail if the file ends unexpectedly
	input.read(content_buffer_bytes, content_buffer_bytes_num);
	ensure_stream_read_success(input);

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
