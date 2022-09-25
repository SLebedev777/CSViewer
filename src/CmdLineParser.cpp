#include "CmdLineParser.h"
#include "Settings.h"
#include <sstream>
#include <algorithm>


// список доступных опций, поддерживаемых программой
const std::vector<CmdLineOptionDescriptionVariant> g_ValidOptions = {
	CmdLineOptionDescriptionChar{ 'H', CmdLineOptionType::CMDLOPT_NOARG, {}, "Print help" },
	CmdLineOptionDescriptionString{ 'E', CmdLineOptionType::CMDLOPT_STR, {"cp1251", "utf8"}, "Set encoding for reading input CSV file"},
	CmdLineOptionDescriptionChar{ 'D', CmdLineOptionType::CMDLOPT_CHAR, {}, "Set delimiter" },
	CmdLineOptionDescriptionChar{ 'Q', CmdLineOptionType::CMDLOPT_CHAR, {'"', '\''}, "Set quoting char to enclose strings"}
};


namespace
{
	void ThrowParseOptionException(const std::string& option_str, const std::string& message = "")
	{
		throw std::runtime_error("Error parsing command line option. " + message + ". Option: " + option_str);
	}

	bool IsTokenOption(const std::string& str, char& key_out)
	{
		if (str.size() < 2)
			return false;

		if ((str[0] == '-') && std::isupper(str[1]))
		{
			key_out = str[1];
			return true;
		}
		return false;
	}

	template <typename Var>
	bool IsOptionExistByKey(const std::vector<Var>& options, char key)
	{
		auto it = std::find_if(options.begin(), options.end(), [&key](const auto& option) {
			char option_key;
			auto key_getter = [&option_key](const auto& option) {
				option_key = option.key;
			};
			std::visit(key_getter, option);
			return option_key == key;
			}
		);
		return it != options.end();
	}

	template <typename Var>
	bool IsHelpOptionExist(const std::vector<Var>& options)
	{
		return IsOptionExistByKey(options, 'H');
	}


	std::optional<CmdLineOptionType> CmdLineOptionKeyToType(char key)
	{
		std::optional<CmdLineOptionType> type;

		auto type_getter = [&type](const auto& var) {
			type = var.type;
		};

		auto it = std::find_if(g_ValidOptions.begin(), g_ValidOptions.end(), [&key](const auto& option) {
			char option_key;
			auto key_getter = [&option_key](const auto& var) {
				option_key = var.key;
			};
			std::visit(key_getter, option);
			return option_key == key;
			}
		);
		if (it != g_ValidOptions.end())
		{
			auto type_getter = [&type](const auto& var) {
				type = var.type;
			};
			std::visit(type_getter, *it);
			return type;
		}
		return {};
	}
}



bool operator==(const CmdLineArgsParseResult& left, const CmdLineArgsParseResult& right)
{
	return (left.input_filename == right.input_filename) && (left.options == right.options);
}


template <typename T>
bool operator==(const CmdLineOptionParseResult<T>& left, const CmdLineOptionParseResult<T>& right)
{
	return (left.key == right.key) && (left.value == right.value);
}


CmdLineOptionParseResultVariant ParseOption(const std::string& option_str)
{
	char key;

	if (!IsTokenOption(option_str, key))
		ThrowParseOptionException(option_str, "Defis not found");

	std::istringstream iss{ option_str };
	char defis;

	iss >> defis >> key;

	auto opt_type = CmdLineOptionKeyToType(key);
	if (!opt_type)
		ThrowParseOptionException(option_str, "Unknown key");

	// empty option check
	if (iss.eof())
	{
		if (*opt_type == CmdLineOptionType::CMDLOPT_NOARG)
			return CmdLineOptionParseResultChar {key, {}};
		else
			ThrowParseOptionException(option_str, "Bad format for non arg option");
	}

	std::string opt_type_str{ std::move(std::to_string(int(*opt_type))) };

	switch (*opt_type)
	{
	case CmdLineOptionType::CMDLOPT_NOARG:
	{
		return CmdLineOptionParseResultChar{ key, {} };
		break;
	}
	case CmdLineOptionType::CMDLOPT_CHAR:
	{
		char c;
		if (iss >> c)
			return CmdLineOptionParseResultChar{ key, c };
		else
			ThrowParseOptionException(option_str, "Type: " + opt_type_str);
		break;
	}
	case CmdLineOptionType::CMDLOPT_STR:
	{
		std::string s;
		if (iss >> s)
			return CmdLineOptionParseResultString{ key, s };
		else
			ThrowParseOptionException(option_str, "Type: " + opt_type_str);
		break;
	}
	case CmdLineOptionType::CMDLOPT_NUM:
	{
		size_t n;
		if (iss >> n)
			return CmdLineOptionParseResultNumber{ key, n };
		else
			ThrowParseOptionException(option_str, "Type: " + opt_type_str);
		break;
	}
	default:
		ThrowParseOptionException(option_str, "Type: " + opt_type_str);
	}

}

CmdLineArgsParseResult ParseCmdLineArgs(int argc, char** argv)
{
	if (argc < 2)
		throw std::runtime_error("Too few command line arguments.");

	std::vector<CmdLineOptionParseResultVariant> parsed_options;
	std::string input_filename;

	bool is_found_input_filename = false;
	for (int i = 1; i < argc; i++)
	{
		std::string token_str(argv[i]);
		char key;
		if (IsTokenOption(token_str, key))
		{
			if (IsOptionExistByKey(parsed_options, key))
				ThrowParseOptionException(token_str, "More than one option with the same key.");

			auto&& parsed_option = ParseOption(token_str);
			parsed_options.push_back(std::move(parsed_option));
		}
		else if (!is_found_input_filename)
		{
			input_filename = std::move(token_str);
			is_found_input_filename = true;
		}
		else
			ThrowParseOptionException(token_str);
	}
	if (!is_found_input_filename && !IsHelpOptionExist(parsed_options))
		throw std::runtime_error("Missing required parameter: input_filename");

	return CmdLineArgsParseResult{ parsed_options, input_filename};
 }