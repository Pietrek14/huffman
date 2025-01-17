#pragma once

#include "../tree/tree.hpp"
#include "../buffer/buffer.hpp"

#include <istream>
#include <ostream>
#include <vector>

namespace Huffman {
	struct EncodedMessage {
		Tree huffman_tree;
		Buffer message_buffer;

		void serialize(std::ostream& output) const;
		static EncodedMessage deserialize(std::istream& input);
	};
};
