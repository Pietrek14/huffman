#pragma once

#include <cstddef>
#include <iterator>
#include <memory>
#include <vector>

namespace Huffman {
	/// @brief A buffer holding raw bit data, used to store the encoded message
	class Buffer {
		std::vector<std::byte> m_Buffer;
		/// @brief How many bits in the final byte of the buffer are used for the content
		uint8_t m_LastByteLength;

	public:
		Buffer();

		Buffer operator<<(bool bit) const;
		void operator<<=(bool bit);
		Buffer operator<<(std::byte byte) const;
		void operator<<=(std::byte byte);
		Buffer operator<<(Buffer buffer) const;
		void operator<<=(Buffer buffer);

		void reserve_bytes(uint32_t bytes_num);

		// Necessary to be used as keys in a dictionary
		bool operator==(const Buffer& other) const;
		size_t hash() const;

		uint64_t get_length() const;

		// Iterators through the stored data are the only way to access it
		class BitIterator {
			std::vector<std::byte>::const_iterator m_BufferIterator;
			uint8_t m_BitIndex;

		public:
			explicit BitIterator(std::vector<std::byte>::const_iterator buffer_iterator, uint8_t bit_index);
			BitIterator& operator++();
			BitIterator operator++(int);
			bool operator==(BitIterator other) const;
			bool operator!=(BitIterator other) const;
			bool operator*() const;
			std::byte next_byte_unsafe();
			std::byte next_byte(const BitIterator& end);

			using iterator_category = std::input_iterator_tag;
			using value_type = bool;
			using difference_type = std::ptrdiff_t;
			using pointer = bool*;
			using reference = bool&;

			class IteratorEndReachedException : public std::exception {
				std::string m_Message;

			public:
				IteratorEndReachedException();

				const char* what() const noexcept override;
			};
		};

		BitIterator bit_begin() const;
		BitIterator bit_end() const;

		std::vector<std::byte>::const_iterator begin() const;
		std::vector<std::byte>::const_iterator end() const;
	};
};

template <>
struct std::hash<Huffman::Buffer>
{
	size_t operator()(const Huffman::Buffer& buffer) const;
};
