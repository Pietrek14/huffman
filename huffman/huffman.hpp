#pragma once

#include <istream>
#include <ostream>

#include "message/message.hpp"

namespace Huffman {
	EncodedMessage encode(std::istream& input);
	void decode(const EncodedMessage& input, std::ostream& output);
}
