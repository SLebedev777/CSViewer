#pragma once

#include <string>
#include <optional>
#include <unordered_map>
#include <variant>
#include <array>
#include <utility>
#include "Settings.h"

// тип значения у опции
enum class CmdLineOptionType
{
	CMDLOPT_NOARG = 0,  // -H -C
	CMDLOPT_CHAR,  // -D, -D; -Q'
	CMDLOPT_STR,  // -Bskip -Ecp1251
	CMDLOPT_NUM  // -S3
};


// результат разбора аргумента командной строки в опцию
// это просто результат парсинга, без привязки к конкретной опции с конкретным описанием и логикой
template <typename T>
struct CmdLineOptionParseResult
{
	const char key;  // ключ опции (первая буква)
	const std::optional<T> value;  // значение, идущее после ключа (может и отсутствовать)
};

using CmdLineOptionParseResultChar = CmdLineOptionParseResult<char>;
using CmdLineOptionParseResultString = CmdLineOptionParseResult<std::string>;
using CmdLineOptionParseResultNumber = CmdLineOptionParseResult<size_t>;


// полиморфный объект, содержащий один из типов распарсенных опций, заданных пользователем
using CmdLineOptionParseResultVariant = std::variant<CmdLineOptionParseResultChar, CmdLineOptionParseResultString, CmdLineOptionParseResultNumber>;

CmdLineOptionParseResultVariant ParseOption(const std::string& option_str);


// логическое описание доступной опции
// чтобы распарсенная строка опции, заданная пользователем, превратилась в валидную опцию, которую программа сможет учесть,
// она должна соответствовать одному из доступных описаний
template <typename T>
struct CmdLineOptionDescription
{
	using value_type = T;

	CmdLineOptionDescription(const char _key, const CmdLineOptionType _type,
		std::vector<T>&& _allowed_values = {}, std::string&& _hint = "")
		: key(_key)
		, type(_type)
		, allowed_values(std::move(_allowed_values))
		, hint(std::move(_hint))
	{}

	const char key;
	const CmdLineOptionType type;
	std::vector<T> allowed_values;
	const std::string hint;
};

using CmdLineOptionDescriptionChar = CmdLineOptionDescription<char>;
using CmdLineOptionDescriptionString = CmdLineOptionDescription<std::string>;
using CmdLineOptionDescriptionNumber = CmdLineOptionDescription<size_t>;


// полиморфный объект, содержащий одну из доступных опций, поддерживаемых программой
using CmdLineOptionDescriptionVariant = std::variant<CmdLineOptionDescriptionChar, CmdLineOptionDescriptionString, CmdLineOptionDescriptionNumber>;

// список доступных опций, поддерживаемых программой
extern const std::vector<CmdLineOptionDescriptionVariant> g_ValidOptions;


template <typename T>
std::ostream& operator<<(std::ostream& os, const std::vector<T> v)
{
	os << "[";
	for (const auto x : v)
		os << x << ", ";
	os << "]";
	return os;
}

template<typename T>
std::ostream& operator<<(std::ostream& os, std::optional<T> const& opt)
{
	return opt ? os << opt.value() : os;
}


auto CmdLineOptionDescriptionVariantPrinter = [](const auto& obj) {
	static const std::string sep = ", ";
	std::cout
		<< "{ "
		<< "Key: " << obj.key << sep
		<< "Type: " << (int)obj.type << sep
		<< "Hint: " << obj.hint << sep
		<< obj.allowed_values
		<< " }"
		<< std::endl;
};

auto CmdLineOptionParseResultVariantPrinter = [](const auto& obj) {
	static const std::string sep = ", ";
	std::cout
		<< "{ "
		<< "Key: " << obj.key << sep
		<< "Value: " << obj.value << sep  // TODO!
		<< " }"
		<< std::endl;
};


// хранит полный результат разбора аргументов командной строки
struct CmdLineArgsParseResult
{
	std::vector<CmdLineOptionParseResultVariant> options;
	std::string input_filename;
};

CmdLineArgsParseResult ParseCmdLineArgs(int argc, char** argv);