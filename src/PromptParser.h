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

bool operator==(const KeywordAndArgs& left, const KeywordAndArgs& right);
bool operator==(const CommandParseResult& left, const CommandParseResult& right);

// �������� ���������� ��������� ����� ������� � ��� ����������
struct CommandKeywordSyntaxDescription
{
	std::vector<CommandKeyword> allowed_kw_values;
	bool is_kw_required;
	std::vector<CommandArgVariant> allowed_args_types;
	size_t min_args;
	size_t max_args;
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

extern const size_t NO_ARGS;
extern const size_t UNLIMITED_ARGS;

// ������� ��������� ������ � ������ ��������� ��������� ����������
extern const std::vector<CommandSyntaxDescription> g_ValidPromptCommands;


class PromptParserException : public std::runtime_error
{
	using std::runtime_error::runtime_error; // inherit base class ctors
};

std::ostream& operator<<(std::ostream& oss, const CommandParseResult& cpr);
