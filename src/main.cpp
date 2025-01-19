#include <iostream>
#include <vector>
#include <stdexcept>

#include "huffman/buffer/buffer.hpp"
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
	}

	return 0;
}