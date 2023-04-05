#pragma once
#include <string>
#include <vector>
#include <map>
#include <variant>
#include <optional>
#include "Command.h"

//class ICommand;
//typedef std::unique_ptr<ICommand> ICommandPtr;

// ����� ��������� ������� :
// command [keyword1] [arg1_1, arg1_2, ...] [keyword2] [arg2_1, arg2_2, ...]

// �������� ����� �������
using CommandKeyword = std::string;

// ���� ���������� �������
// ������������ ����
using CommandArgString = std::string;  // ������
using CommandArgNumber = int;  // �����
using CommandArgPrimitiveType = std::variant<CommandArgString, CommandArgNumber>;
// ��������� ����
using CommandArgStringRange = std::pair<CommandArgString, CommandArgString>;  // �������� ������1:������2
using CommandArgNumberRange = std::pair<CommandArgNumber, CommandArgNumber>;  // �������� �����1:�����2
using CommandArgKeyValuePair = std::pair<CommandArgPrimitiveType, CommandArgPrimitiveType>;  // ����-��������

// ����������� ������ ��������� �������, �������� �������� ������ �� ��������� �����
using CommandArgVariant = std::variant<
	CommandArgString,
	CommandArgNumber,
	CommandArgStringRange,
	CommandArgNumberRange,
	CommandArgKeyValuePair
>;


// ����������������� ��������� ������� ��������� � ��������� �������
struct CommandParseResult
{
	std::string command;  // ���� ������� - ������ ����� � ��������� ������ � ������� - ��� ������ ������
	CommandKeyword kw1;  // �������� �����1
	CommandKeyword kw2;  // �������� �����2
	std::vector<CommandArgVariant> args1;  // ��������� � ��������� �����1
	std::vector<CommandArgVariant> args2;  // ��������� � ��������� �����2
};


std::optional<CommandArgPrimitiveType> ParseTokenToPrimitiveType(const std::string& token);
CommandArgVariant ParseToken(const std::string& token);
CommandParseResult ParsePromptInput(const std::string& prompt_input);

extern const char COMMAND_ARG_RANGE_DELIMITER;
extern const char COMMAND_ARG_KEYVALUE_DELIMITER;

