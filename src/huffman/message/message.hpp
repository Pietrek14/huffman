#pragma once

#include "../tree/tree.hpp"
#include "../buffer/buffer.hpp"

#include <istream>
#include <ostream>
#include <stdexcept>
#include <vector>

namespace Huffman {
	struct EncodedMessage {
		Tree huffman_tree;
		Buffer message_buffer;

		void serialize(std::ostream& output) const;
		static EncodedMessage deserialize(std::istream& input);

	private:
		/// @brief A helper function for the `deserialize` method
		/// @param stream The input stream (usually a file)
		static void ensure_stream_read_success(std::istream& stream);

	public:

		class DeserializationException : public std::exception {
		protected:
			std::string m_Message;

			DeserializationException();

		public:
			const char* what() const noexcept override;
		};

		class UnexpectedEofException : public DeserializationException {
		public:
			UnexpectedEofException();
		};

		class InvalidHeaderException : public DeserializationException {
			std::string m_FileHeader;
			std::string m_ExpectedHeader;

		public:
			InvalidHeaderException(std::string file_header, std::string expected_header);

			std::string get_file_header() const;
			std::string get_expected_header() const;
		};

		class WrongVersionException : public DeserializationException {
			uint8_t m_FileVersion;
			uint8_t m_SoftwareVersion;

		public:
			WrongVersionException(uint8_t file_version, uint8_t software_version);

			uint8_t get_file_version() const;
			uint8_t get_software_version() const;
		};

		class InvalidTreeDataException : public DeserializationException {
		public:
			InvalidTreeDataException();
		};

		class InvalidFooterException : public DeserializationException {
			std::string m_FileFooter;
			std::string m_ExpectedFooter;

		public:
			InvalidFooterException(std::string file_footer, std::string expected_footer);

			std::string get_file_footer() const;
			std::string get_expected_footer() const;
		};
	};
};
