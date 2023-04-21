#pragma once
#include <string>
#include <vector>
#include <map>
#include <variant>
#include <optional>
#include <unordered_map>
#include "Command.h"

//class ICommand;
//typedef std::unique_ptr<ICommand> ICommandPtr;

// ����� ��������� ������� :
// command [keyword1] [arg1_1, arg1_2, ...] [|] [keyword2] [arg2_1, arg2_2, ...]
// | - ������-�����������, ���������� ��������� ��������� ����� 1 �� ��������� �����2
 

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


enum class CommandArgEnumType
{
	CMD_ARG_STRING = 0,
	CMD_ARG_NUMBER,
	CMD_ARG_STRING_RANGE,
	CMD_ARG_NUMBER_RANGE,
	CMD_ARG_KEY_VALUE_PAIR,
	CMD_ARG_UNKNOWN_TYPE = -1
};

struct KeywordAndArgs
{
	CommandKeyword kw;
	std::vector<CommandArgVariant> args;
};

// ����������������� ��������� ������� ��������� � ��������� �������
struct CommandParseResult
{
	std::string command;  // ���� ������� - ������ ����� � ��������� ������ � ������� - ��� ������ ������
	std::vector<KeywordAndArgs> keywords_and_args;
};

// �������� ���������� ��������� ����� ������� � ��� ����������
struct CommandKeywordSyntaxDescription
{
	std::vector<CommandKeyword> allowed_kw_values;
	bool is_kw_required;
	std::vector<CommandArgVariant> allowed_args_types;
	bool is_args_required;
};

struct CommandSyntaxDescription
{
	std::string command_full;
	std::string command_short;
	std::vector<std::vector<CommandKeywordSyntaxDescription>> keywords_and_args;  // ������ ��������� ���������� �������
};

bool operator==(const CommandKeywordSyntaxDescription& left, const CommandKeywordSyntaxDescription& right);
bool operator==(const CommandSyntaxDescription& left, const CommandSyntaxDescription& right);


std::optional<CommandArgPrimitiveType> ParseTokenToPrimitiveType(const std::string& token);
CommandArgVariant ParseToken(const std::string& token);
CommandParseResult ParsePromptInput(const std::string& prompt_input);

extern const char COMMAND_ARG_RANGE_DELIMITER;
extern const char COMMAND_ARG_KEYVALUE_DELIMITER;
extern const char COMMAND_KEYWORD_DELIMITER;
extern const char COMMAND_ARGS_DELIMITER;

// ������� ��������� ������ � ������ ��������� ��������� ����������
extern const std::vector<CommandSyntaxDescription> g_ValidPromptCommands;