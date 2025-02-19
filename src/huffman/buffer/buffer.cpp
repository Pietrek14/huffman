#include "buffer.hpp"

Huffman::Buffer::Buffer()
	: m_Buffer(std::vector<std::byte>(1, std::byte(0))), m_LastByteLength(0) {}

void Huffman::Buffer::operator<<=(bool bit) {
	if(m_LastByteLength == 8) {
		m_Buffer.push_back(std::byte(bit) << 7);
		m_LastByteLength = 1;

		return;
	}

	std::byte& last = m_Buffer[m_Buffer.size() - 1];

	last |= (std::byte)bit << (7 - m_LastByteLength);

	m_LastByteLength++;
}

Huffman::Buffer Huffman::Buffer::operator<<(bool bit) const {
	Buffer result = *this;

	result <<= bit;

	return result;
}

void Huffman::Buffer::operator<<=(std::byte byte) {
	std::byte& last = m_Buffer[m_Buffer.size() - 1];

	last |= byte >> m_LastByteLength;
	m_Buffer.push_back(byte << (8 - m_LastByteLength));
}

Huffman::Buffer Huffman::Buffer::operator<<(std::byte byte) const {
	Buffer result = *this;

	result <<= byte;

	return result;
}

void Huffman::Buffer::operator<<=(Buffer buffer) {
	// TODO: There are quicker ways to do this (although more complicated)
	for(auto it = buffer.bit_begin(); it != buffer.bit_end(); it++) {
		*this <<= *it;
	}
}

Huffman::Buffer Huffman::Buffer::operator<<(Buffer buffer) const {
	Buffer result = *this;

	result <<= buffer;

	return result;
}

bool Huffman::Buffer::operator==(const Buffer& buffer) const {
	if(get_length() != buffer.get_length()) {
		return false;
	}

	for(uint32_t i = 0; i < m_Buffer.size(); i++) {
		if(m_Buffer[i] != buffer.m_Buffer[i])
			return false;
	}

	return true;
}

void Huffman::Buffer::reserve_bytes(uint32_t bytes_num) {
	m_Buffer.reserve(bytes_num);
}

size_t Huffman::Buffer::hash() const {
	size_t seed = m_Buffer.size();

	for(std::byte byte : m_Buffer) {
		seed ^= std::to_integer<uint8_t>(byte) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
	}

	return seed;
}

size_t std::hash<Huffman::Buffer>::operator()(const Huffman::Buffer& buffer) const {
	return buffer.hash();
}

uint64_t Huffman::Buffer::get_length() const {
	return (m_Buffer.size() - 1) * 8 + m_LastByteLength;
}

Huffman::Buffer::BitIterator Huffman::Buffer::bit_begin() const {
	return BitIterator(m_Buffer.begin(), 0);
}

Huffman::Buffer::BitIterator Huffman::Buffer::bit_end() const {
	if(m_LastByteLength == 8) {
		return BitIterator(m_Buffer.end(), 0);
	}

	return BitIterator(m_Buffer.end() - 1, m_LastByteLength);
}

std::vector<std::byte>::const_iterator Huffman::Buffer::begin() const {
	return m_Buffer.begin();
}

std::vector<std::byte>::const_iterator Huffman::Buffer::end() const {
	return m_Buffer.end();
}

// Iterator definitions
Huffman::Buffer::BitIterator::BitIterator(std::vector<std::byte>::const_iterator buffer_iterator, uint8_t bit_index)
	: m_BufferIterator(buffer_iterator), m_BitIndex(bit_index) {}

Huffman::Buffer::BitIterator& Huffman::Buffer::BitIterator::operator++() {
	m_BitIndex++;

	if(m_BitIndex == 8) {
		m_BitIndex = 0;
		++m_BufferIterator;
	}

	return *this;
}

Huffman::Buffer::BitIterator Huffman::Buffer::BitIterator::operator++(int) {
	BitIterator result = *this;

	++(*this);

	return result;
}

bool Huffman::Buffer::BitIterator::operator==(Huffman::Buffer::BitIterator other) const {
	return std::distance(m_BufferIterator, other.m_BufferIterator) == 0 && m_BitIndex == other.m_BitIndex;
}

bool Huffman::Buffer::BitIterator::operator!=(Huffman::Buffer::BitIterator other) const {
	return !(*this == other);
}

bool Huffman::Buffer::BitIterator::operator*() const {
	uint8_t num = (int)(*m_BufferIterator);

	return (bool)((std::byte)(1 << (7 - m_BitIndex)) & (*m_BufferIterator));
}

std::byte Huffman::Buffer::BitIterator::next_byte_unsafe() {
	// TODO: Optimize this to use two bytes and shifing instead of going over all bits seperately
	std::byte byte = std::byte(0);

	for(uint8_t i = 0; i < 8; i++) {
		byte |= std::byte(*(*this) << (7 - i));
		++(*this);
	}

	return byte;
}

std::byte Huffman::Buffer::BitIterator::next_byte(const BitIterator& end) {
	// TODO: Optimize this to use two bytes and shifing instead of going over all bits seperately
	std::byte byte = std::byte(0);

	for(uint8_t i = 0; i < 8; i++) {
		byte |= std::byte(*(*this) << (7 - i));

		if(*this == end) {
			throw IteratorEndReachedException();
		}

		++(*this);
	}

	return byte;
}

Huffman::Buffer::BitIterator::IteratorEndReachedException::IteratorEndReachedException() {
	m_Message = "The iterator reached its end unexpectedly.";
}

const char* Huffman::Buffer::BitIterator::IteratorEndReachedException::what() const noexcept {
	return m_Message.c_str();
}
