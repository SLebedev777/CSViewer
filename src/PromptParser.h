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

// ќбщий синтаксис команды :
// command [keyword1] [arg1_1, arg1_2, ...] [|] [keyword2] [arg2_1, arg2_2, ...]
// | - символ-разделитель, отдел€юший аргументы ключевого слова 1 от ключевого слова2
 

// ключевое слово команды
using CommandKeyword = std::string;

// типы аргументов команды
// элементарные типы
using CommandArgString = std::string;  // строка
using CommandArgNumber = int;  // число
using CommandArgPrimitiveType = std::variant<CommandArgString, CommandArgNumber>;
// составные типы
using CommandArgStringRange = std::pair<CommandArgString, CommandArgString>;  // диапазон строка1:строка2
using CommandArgNumberRange = std::pair<CommandArgNumber, CommandArgNumber>;  // диапазон число1:число2
using CommandArgKeyValuePair = std::pair<CommandArgPrimitiveType, CommandArgPrimitiveType>;  // ключ-значение

// полиморфный объект аргумента команды, хран€щий аргумент одного из доступных типов
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

// структурированный результат разбора введенной с терминала команды
struct CommandParseResult
{
	std::string command;  // сама команда - первый токен в введенной строке с консоли - это всегда строка
	std::vector<KeywordAndArgs> keywords_and_args;
};

bool operator==(const KeywordAndArgs& left, const KeywordAndArgs& right);
bool operator==(const CommandParseResult& left, const CommandParseResult& right);

// описание синтаксиса ключевого слова команды и его аргументов
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
	std::vector<std::vector<CommandKeywordSyntaxDescription>> keywords_and_args;  // вектор вариантов синтаксиса команды
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

// словарь доступных команд с полным описанием вариантов синтаксиса
extern const std::vector<CommandSyntaxDescription> g_ValidPromptCommands;


class PromptParserException : public std::runtime_error
{
	using std::runtime_error::runtime_error; // inherit base class ctors
};
