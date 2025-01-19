#include "interface.hpp"

#include <iostream>
#include <fstream>

#include "huffman/huffman.hpp"

Action::Action(const std::vector<std::string>& args) {
	if(args.size() < 1) {
		throw IncorrectCallException();
	}

	if(args.size() < 2) {
		// throw std::runtime_error("AAAA");
		throw NoActionException();
	}

	std::string action_name = args[1];

	if(action_name == "decode" || action_name == "d") {
		m_Type = ActionType::Decode;
	} else if(action_name == "encode" || action_name == "e") {
		m_Type = ActionType::Encode;
	} else if(action_name == "test" || action_name == "t") {
		m_Type = ActionType::Test;
	} else if(action_name == "help" || action_name == "h") {
		m_Type = ActionType::Help;
	} else {
		throw UnknownActionException(action_name);
	}

	m_Args = std::vector<std::string>(args.size() - 2);

	for(uint16_t i = 2; i < args.size(); i++) {
		m_Args[i - 2] = args[i];
	}

	if(m_Args.size() != expected_arg_count(m_Type)) {
		throw WrongArgumentCountException(m_Type, m_Args.size());
	}
}

std::string Action::action_name(ActionType action_type) {
	switch(action_type) {
	case ActionType::Decode:
		return "decode";

	case ActionType::Encode:
		return "encode";

	case ActionType::Test:
		return "test";

	case ActionType::Help:
		return "help";
	}

	return "unknown";
}

uint16_t Action::expected_arg_count(ActionType action_type) {
	switch(action_type) {
	case ActionType::Decode:
		return 1;

	case ActionType::Encode:
		return 2;

	case ActionType::Test:
		return 1;

	case ActionType::Help:
		return 0;
	}

	return 0;
}

void Action::perform() const {
	switch(m_Type) {
	case ActionType::Decode:
		decode();
		break;

	case ActionType::Encode:
		encode();
		break;

	case ActionType::Test:
		test();
		break;

	case ActionType::Help:
		help();
		break;
	}
}

void Action::decode() const {
	std::ifstream input(m_Args[0], std::ios::binary | std::ios::in);

	if(!input.good()) {
		throw FailedFileReadException(m_Args[0]);
	}

	auto deserialized_message = Huffman::EncodedMessage::deserialize(input);

	input.close();

	Huffman::decode(deserialized_message, std::cout);
}

void Action::encode() const {
	std::ifstream input(m_Args[0], std::ios::binary | std::ios::in);

	if(!input.good()) {
		throw FailedFileReadException(m_Args[0]);
	}

	auto message = Huffman::encode(input);

	input.close();

	std::ofstream output(m_Args[1], std::ios::binary | std::ios::out);

	if(!output.good()) {
		throw FailedFileWriteException(m_Args[1]);
	}

	message.serialize(output);

	output.close();
}

void Action::test() const {
	std::ifstream input(m_Args[0], std::ios::binary | std::ios::in);

	if(!input.good()) {
		throw FailedFileReadException(m_Args[0]);
	}

	auto message = Huffman::encode(input);

	input.close();

	Huffman::decode(message, std::cout);
}

void Action::help() const {
	std::cout
		<< " ___  ___  ________ ________ \n"
		<< "|\\  \\|\\  \\|\\  _____\\  _____\\\n"
		<< "\\ \\  \\\\\\  \\ \\  \\__/\\ \\  \\__/ \n"
		<< " \\ \\   __  \\ \\   __\\\\ \\   __\\\n"
		<< "  \\ \\  \\ \\  \\ \\  \\_| \\ \\  \\_|\n"
		<< "   \\ \\__\\ \\__\\ \\__\\   \\ \\__\\ \n"
		<< "    \\|__|\\|__|\\|__|    \\|__| \n\n"

		<< "Usage: hff.exe <command> <args>\n"
		<< "Available commands:\n"

		<< "\tencode\n"
		<< "\t\targs: <input file> <output file>\n"
		<< "\t\tencodes the input file using Huffman encoding"
		<< "and serializes the results into the output file.\n"

		<< "\tdecode\n"
		<< "\t\targs: <input file>\n"
		<< "\t\tdecodes the input file serialized with the encode command"
		<< "and outputs the results into cmd.\n"

		<< "\thelp\n"
		<< "\t\targs: none\n"
		<< "\t\tdisplays this\n";
}

// Exceptions
Action::Exception::Exception() {}

const char* Action::Exception::what() const noexcept {
	return m_Message.c_str();
}

Action::IncorrectCallException::IncorrectCallException() {
	m_Message = "Action expects the args vector to have at least one element. Make sure you're constructing it with raw cmd args.";
}

Action::NoActionException::NoActionException() {
	m_Message = "No action called!";
}

Action::UnknownActionException::UnknownActionException(std::string action_name) {
	m_ActionName = action_name;
	m_Message = "Unknown action name: " + action_name;
}

std::string Action::UnknownActionException::get_action_name() const {
	return m_ActionName;
}

Action::WrongArgumentCountException::WrongArgumentCountException(ActionType action_type, uint16_t arg_count) {
	m_ActionType = action_type;
	m_ArgCount = arg_count;
	m_Message = "Action " + action_name(action_type) + " expects " + std::to_string(expected_arg_count(action_type)) + " args, got " + std::to_string(arg_count);
}

Action::ActionType Action::WrongArgumentCountException::get_action_type() const {
	return m_ActionType;
}

uint16_t Action::WrongArgumentCountException::get_argument_count() const {
	return m_ArgCount;
}

Action::FailedFileReadException::FailedFileReadException(std::string filename) {
	m_Filename = filename;
	m_Message = "Couldn't read from file '" + filename + "'. Make sure it exists and you have the necessary permissions.";
}

std::string Action::FailedFileReadException::get_filename() const {
	return m_Filename;
}

Action::FailedFileWriteException::FailedFileWriteException(std::string filename) {
	m_Filename = filename;
	m_Message = "Couldn't write to file '" + filename + "'. Make sure you have the necessary permissions.";
}

std::string Action::FailedFileWriteException::get_filename() const {
	return m_Filename;
}