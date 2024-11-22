#pragma once

#include <istream>
#include <ostream>

#include "message/message.hpp"

namespace Huffman {
	EncodedMessage encode(std::istream& input);
	std::ostream decode(std::istream& input);
}
