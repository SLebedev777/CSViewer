#pragma once

#include <string>
#include <optional>
#include <unordered_map>
#include <variant>
#include <array>
#include <utility>
#include "Settings.h"

// ��� �������� � �����
enum class CmdLineOptionType
{
	CMDLOPT_NOARG = 0,  // -H -C
	CMDLOPT_CHAR,  // -D, -D; -Q'
	CMDLOPT_STR,  // -Bskip -Ecp1251
	CMDLOPT_NUM  // -S3
};


// ��������� ������� ��������� ��������� ������ � �����
// ��� ������ ��������� ��������, ��� �������� � ���������� ����� � ���������� ��������� � �������
template <typename T>
struct CmdLineOptionParseResult
{
	const char key;  // ���� ����� (������ �����)
	const std::optional<T> value;  // ��������, ������ ����� ����� (����� � �������������)
};

using CmdLineOptionParseResultChar = CmdLineOptionParseResult<char>;
using CmdLineOptionParseResultString = CmdLineOptionParseResult<std::string>;
using CmdLineOptionParseResultNumber = CmdLineOptionParseResult<size_t>;

template <typename T>
bool operator==(const CmdLineOptionParseResult<T>& left, const CmdLineOptionParseResult<T>& right);

// ����������� ������, ���������� ���� �� ����� ������������ �����, �������� �������������
using CmdLineOptionParseResultVariant = std::variant<CmdLineOptionParseResultChar, CmdLineOptionParseResultString, CmdLineOptionParseResultNumber>;

CmdLineOptionParseResultVariant ParseOption(const std::string& option_str);


// ���������� �������� ��������� �����
// ����� ������������ ������ �����, �������� �������������, ������������ � �������� �����, ������� ��������� ������ ������,
// ��� ������ ��������������� ������ �� ��������� ��������
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


// ����������� ������, ���������� ���� �� ��������� �����, �������������� ����������
using CmdLineOptionDescriptionVariant = std::variant<CmdLineOptionDescriptionChar, CmdLineOptionDescriptionString, CmdLineOptionDescriptionNumber>;

// ������ ��������� �����, �������������� ����������
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


// ������ ������ ��������� ������� ���������� ��������� ������
struct CmdLineArgsParseResult
{
	std::vector<CmdLineOptionParseResultVariant> options;
	std::string input_filename;
};

bool operator==(const CmdLineArgsParseResult& left, const CmdLineArgsParseResult& right);


CmdLineArgsParseResult ParseCmdLineArgs(int argc, char** argv);


namespace csviewer_internal
{
	void ThrowParseOptionException(const std::string& option_str, const std::string& message = "");

	bool IsTokenOption(const std::string& str, char& key_out);

	template <typename Var>
	auto FindOptionByKey(const std::vector<Var>& options, char key);

	template <typename Var>
	bool IsOptionExistByKey(const std::vector<Var>& options, char key);

	template <typename Var>
	bool IsHelpOptionExist(const std::vector<Var>& options);

	std::optional<CmdLineOptionType> CmdLineOptionKeyToType(char key);

	template<typename T>
	bool IsOptionValueAllowed(const CmdLineOptionDescriptionVariant& var, const T& value);
}
