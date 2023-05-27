#pragma once

#include <string>
#include <optional>
#include <unordered_map>
#include <variant>
#include <array>
#include <utility>
#include "CSV.h"


// тип значения у опции
enum class CmdLineOptionType
{
	CMDLOPT_NOARG = 0,  // -H -C
	CMDLOPT_CHAR,  // -D, -D; -Q'
	CMDLOPT_STR,  // -Bskip -Ecp1251
	CMDLOPT_NUM  // -S3
};

using OptionKey = char;

// результат разбора аргумента командной строки в опцию
// это просто результат парсинга, без привязки к конкретной опции с конкретным описанием и логикой
template <typename T>
struct CmdLineOptionParseResult
{
	const OptionKey key;  // ключ опции (первая буква)
	const std::optional<T> value;  // значение, идущее после ключа (может и отсутствовать)
};

using CmdLineOptionParseResultChar = CmdLineOptionParseResult<char>;
using CmdLineOptionParseResultString = CmdLineOptionParseResult<std::string>;
using CmdLineOptionParseResultNumber = CmdLineOptionParseResult<size_t>;

template <typename T>
bool operator==(const CmdLineOptionParseResult<T>& left, const CmdLineOptionParseResult<T>& right);

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

	CmdLineOptionDescription(const OptionKey& _key, const CmdLineOptionType _type,
		std::vector<T>&& _allowed_values = {}, std::string&& _hint = "")
		: key(_key)
		, type(_type)
		, allowed_values(std::move(_allowed_values))
		, hint(std::move(_hint))
	{}

	const OptionKey key;
	const CmdLineOptionType type;
	std::vector<T> allowed_values;
	const std::string hint;
};

using CmdLineOptionDescriptionChar = CmdLineOptionDescription<char>;
using CmdLineOptionDescriptionString = CmdLineOptionDescription<std::string>;
using CmdLineOptionDescriptionNumber = CmdLineOptionDescription<size_t>;


// полиморфный объект, содержащий одну из доступных опций, поддерживаемых программой
using CmdLineOptionDescriptionVariant = std::variant<CmdLineOptionDescriptionChar, CmdLineOptionDescriptionString, CmdLineOptionDescriptionNumber>;

using CmdLineOptionDescriptionContainer = std::unordered_map<OptionKey, CmdLineOptionDescriptionVariant>;

// словарь доступных опций, поддерживаемых программой
extern const CmdLineOptionDescriptionContainer g_ValidOptions;


template <typename T>
std::ostream& operator<<(std::ostream& os, const std::vector<T>& v)
{
	os << "[";
	for (const auto& x : v)
		os << x << ", ";
	os << "]";
	return os;
}

template<typename T>
std::ostream& operator<<(std::ostream& os, std::optional<T> const& opt)
{
	return opt ? os << opt.value() : os;
}


inline auto CmdLineOptionDescriptionVariantPrinter = [](const auto& obj) {
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

inline auto CmdLineOptionParseResultVariantPrinter = [](const auto& obj) {
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

bool operator==(const CmdLineArgsParseResult& left, const CmdLineArgsParseResult& right);


CmdLineArgsParseResult ParseCmdLineArgs(int argc, char** argv);

// CSV settings factory function from cmd line args
CSVLoadingSettings MakeSettingsByCmdLineArgs(const CmdLineArgsParseResult& cmd_line_args);


namespace csviewer_internal
{
	void ThrowParseOptionException(const std::string& option_str, const std::string& message = "");

	bool IsTokenOption(const std::string& str, OptionKey& key_out);

	std::optional<CmdLineOptionDescriptionVariant> FindOptionByKey(const CmdLineOptionDescriptionContainer& options, OptionKey key);

	bool IsOptionExistByKey(const CmdLineOptionDescriptionContainer& options, OptionKey key);

	template <typename Var>
	bool IsOptionExistByKey(const std::vector<Var>& options, OptionKey key);

	template <typename Var>
	bool IsHelpOptionExist(const std::vector<Var>& options);

	std::optional<CmdLineOptionType> CmdLineOptionKeyToType(const CmdLineOptionDescriptionContainer& options, OptionKey key);

	template<typename T>
	bool IsOptionValueAllowed(const CmdLineOptionDescriptionVariant& var, const T& value);
}
