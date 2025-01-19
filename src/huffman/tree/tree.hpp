#pragma once

#include "../buffer/buffer.hpp"

#include <memory>
#include <unordered_map>
#include <vector>

namespace Huffman {
	class Tree {
		class Node {
			std::unique_ptr<Node> m_Left;
			std::unique_ptr<Node> m_Right;

			uint64_t m_Occurences;
			uint8_t m_Character;
		public:
			Node(uint8_t character, uint64_t occurences);

			void push_left(std::unique_ptr<Node>&& node);
			void push_right(std::unique_ptr<Node>&& node);

			uint8_t get_character() const;
			uint64_t get_occurances() const;
			const std::unique_ptr<Node>& get_left() const;
			const std::unique_ptr<Node>& get_right() const;
		};

		std::unique_ptr<Node> m_Root;

	public:
		using Code = Buffer;
		using CodeDictionary = std::unordered_map<uint8_t, Code>;
		using CharacterDictionary = std::unordered_map<Code, uint8_t>;

		/// @brief Initialize a single-node Huffman tree
		Tree(uint8_t character, uint64_t occurences);

		/// @brief Fuses two trees together under one root
		Tree(Tree&& left_tree, Tree&& right_tree);

		uint64_t get_occurances() const;
		CodeDictionary get_codes() const;
		CharacterDictionary get_codes_for_decoding() const;

		/// @brief Serializes the tree using preorder traversal
		/// @return A buffer which the tree is serialized into
		Buffer serialize() const;

		/// @brief Deserializes a tree serialized using the `serialize` method
		/// @param buffer A buffer containing the serialized data
		/// @return A tree constructed from the serialized data
		static Tree deserialize(const Buffer& buffer);

	private:
		// A helper function for the `get_codes` method
		void generate_codes(CodeDictionary& code, const std::unique_ptr<Node>& current_node, Code current_code) const;

		// A helper function for the `get_codes_for_decoding` method
		void generate_codes_for_decoding(CharacterDictionary& code, const std::unique_ptr<Node>& current_node, Code current_code) const;

		// A helper function for the `serialize` method
		void preorder_serialization(Buffer& output, const std::unique_ptr<Node>& current_node) const;

		// A helper function for the `deserialize` method
		static void preorder_deserialization(std::unique_ptr<Node>& root, Buffer::BitIterator& input, const Buffer::BitIterator& end);
	};
};
