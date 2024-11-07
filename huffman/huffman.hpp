#pragma once

#include <istream>
#include <ostream>

namespace Huffman {
	std::ostream encode(std::istream& input);
	std::ostream decode(std::istream& input);
}
