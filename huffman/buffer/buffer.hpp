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

			using iterator_category = std::input_iterator_tag;
			using value_type = bool;
			using difference_type = std::ptrdiff_t;
			using pointer = bool*;
			using reference = bool&;
		};

		BitIterator bit_begin() const;
		BitIterator bit_end() const;

		std::vector<std::byte>::const_iterator begin() const;
		std::vector<std::byte>::const_iterator end() const;
	};
};