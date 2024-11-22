#pragma once

#include "../tree/tree.hpp"
#include "../buffer/buffer.hpp"

#include <istream>
#include <ostream>
#include <vector>

namespace Huffman {
	struct EncodedMessage {
		Tree huffmanTree;
		Buffer messageBuffer;

		std::ostream serialize() const;
		static EncodedMessage deserialize(std::istream input);
	};
};
