#include <iostream>
#include <vector>
#include <stdexcept>

#include "huffman/huffman.hpp"
#include "huffman/message/message.hpp"
#include "interface.hpp"

int main(int argc, const char* argv[]) {
	std::vector<std::string> args(argc);

	for(uint16_t i = 0; i < argc; i++) {
		args[i] = argv[i];
	}

	try {
		const Action action(args);

		action.perform();
	} catch(const Action::NoActionException& e) {
		std::cerr << "No command given! For a list of available commands use `hff.exe help`.\n";

		return 1;
	} catch(const Action::UnknownActionException& e) {
		std::cerr << "Unknown command: '" << e.get_action_name() << "'. For a list of available commands use `hff.exe help`\n";

		return 1;
	} catch(const Action::WrongArgumentCountException& e) {
		auto expected_arg_count = Action::expected_arg_count(e.get_action_type());

		std::cerr << "Command '" << Action::action_name(e.get_action_type())
			<< "' expects " << expected_arg_count << (Action::expected_arg_count(e.get_action_type()) == 1 ? " argument" : " arguments")
			<< ", supplied with " << e.get_argument_count() << ".\n";

		return 1;
	} catch(const Action::FailedFileReadException& e) {
		std::cerr << "Failed to read from file '" << e.get_filename() << "'. Make sure it exists and you have the necessary permissions.\n";

		return 1;
	} catch(const Action::FailedFileWriteException& e) {
		std::cerr << "Failed to write to file '" << e.get_filename() << "'. Make sure you have the necessary permissions.\n";

		return 1;
	} catch(const Huffman::OneCharacterSourceException& e) {
		std::cerr << "Encoding single character sequences is currently not supported.\n";

		return 1;
	} catch(const Huffman::EncodedMessage::UnexpectedEofException& e) {
		std::cerr << "The file ended unexpectedly. Make sure you have the entire file.\n";

		return 1;
	} catch(const Huffman::EncodedMessage::InvalidHeaderException& e) {
		std::cerr
			<< "Invalid file header. Expected '" << e.get_expected_header() << "', got '" << e.get_file_header()
			<< "'. Given file may not be a hff file.\n";

		return 1;
	} catch(const Huffman::EncodedMessage::WrongVersionException& e) {
		std::cerr
			<< "Your software is out of date. File was encoded with ver " << static_cast<uint32_t>(e.get_file_version())
			<< ". You're using ver " << static_cast<uint32_t>(e.get_software_version()) << ".\n";

		return 1;
	} catch(const Huffman::EncodedMessage::InvalidTreeDataException& e) {
		std::cerr << "The tree data is invalid. Given file may be corrupted.\n";

		return 1;
	} catch(const Huffman::EncodedMessage::InvalidFooterException& e) {
		std::cerr << "Invalid file footer. Expected '" << e.get_expected_footer()
			<< "', got '" << e.get_file_footer() << "'. You can try fixing the error "
			<< "by changing the last two characters to '" << e.get_expected_footer()
			<< "', but an invalid footer suggests a corrupted file.\n";

		return 1;
	}

	return 0;
}