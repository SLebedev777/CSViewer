#pragma once
#include <string>
#include <vector>
#include <map>
#include <variant>
#include <optional>
#include "Command.h"

//class ICommand;
//typedef std::unique_ptr<ICommand> ICommandPtr;

// Общий синтаксис команды :
// command [keyword1] [arg1_1, arg1_2, ...] [keyword2] [arg2_1, arg2_2, ...]

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

// полиморфный объект аргумента команды, хранящий аргумент одного из доступных типов
using CommandArgVariant = std::variant<
	CommandArgString,
	CommandArgNumber,
	CommandArgStringRange,
	CommandArgNumberRange,
	CommandArgKeyValuePair
>;


// структурированный результат разбора введенной с терминала команды
struct CommandParseResult
{
	std::string command;  // сама команда - первый токен в введенной строке с консоли - это всегда строка
	CommandKeyword kw1;  // ключевое слово1
	CommandKeyword kw2;  // ключевое слово2
	std::vector<CommandArgVariant> args1;  // аргументы к ключевому слову1
	std::vector<CommandArgVariant> args2;  // аргументы к ключевому слову2
};


std::optional<CommandArgPrimitiveType> ParseTokenToPrimitiveType(const std::string& token);
CommandArgVariant ParseToken(const std::string& token);
CommandParseResult ParsePromptInput(const std::string& prompt_input);

extern const char COMMAND_ARG_RANGE_DELIMITER;
extern const char COMMAND_ARG_KEYVALUE_DELIMITER;

