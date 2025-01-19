#pragma once

#include <stdexcept>
#include <string>
#include <vector>

class Action {
	enum class ActionType {
		Encode,
		Decode,
		Test,
		Help
	};

	ActionType m_Type;
	std::vector<std::string> m_Args;

	void encode() const;
	void decode() const;
	void test() const;
	void help() const;

public:
	static std::string action_name(ActionType action_type);
	static uint16_t expected_arg_count(ActionType action_type);

	Action(const std::vector<std::string>& args);

	void perform() const;

	// Exception types
private:
	class Exception : public std::exception {
	protected:
		std::string m_Message;

		Exception();

	public:
		const char* what() const noexcept override;
	};

public:
	class IncorrectCallException : public Exception {
	public:
		IncorrectCallException();
	};

	class NoActionException : Exception {
	public:
		NoActionException();
	};

	class UnknownActionException : public Exception {
		std::string m_ActionName;
	public:
		UnknownActionException(std::string action_name);

		std::string get_action_name() const;
	};

	class WrongArgumentCountException : public Exception {
		ActionType m_ActionType;
		uint16_t m_ArgCount;
	public:
		WrongArgumentCountException(ActionType action_type, uint16_t arg_count);

		ActionType get_action_type() const;
		uint16_t get_argument_count() const;
	};

	class FailedFileReadException : public Exception {
		std::string m_Filename;
	public:
		FailedFileReadException(std::string filename);

		std::string get_filename() const;
	};

	class FailedFileWriteException : public Exception {
		std::string m_Filename;
	public:
		FailedFileWriteException(std::string filename);

		std::string get_filename() const;
	};
};
