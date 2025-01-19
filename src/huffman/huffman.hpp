#pragma once

#include <istream>
#include <ostream>
#include <stdexcept>
#include <string>

#include "message/message.hpp"

namespace Huffman {
	EncodedMessage encode(std::istream& input);
	void decode(const EncodedMessage& input, std::ostream& output);

	class OneCharacterSourceException : public std::exception {
	public:
		const char* what() const noexcept override;
	};
}
