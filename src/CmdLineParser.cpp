#include "CmdLineParser.h"
#include <sstream>
#include <algorithm>
#include "CSV.h"


// ������� ��������� �����, �������������� ����������
const CmdLineOptionDescriptionContainer g_ValidOptions = {
	{'H', CmdLineOptionDescriptionChar{'H', CmdLineOptionType::CMDLOPT_NOARG, {}, "Print help"}},
	{'E', CmdLineOptionDescriptionString{'E', CmdLineOptionType::CMDLOPT_STR, {"cp1251", "utf8", "cp866"}, "Set encoding for reading input CSV file"}},
	{'D', CmdLineOptionDescriptionChar{'D', CmdLineOptionType::CMDLOPT_CHAR, {}, "Set delimiter"}},
	{'Q', CmdLineOptionDescriptionChar{'Q', CmdLineOptionType::CMDLOPT_CHAR, {'"', '\''}, "Set quoting char to enclose strings"}},
	{'C', CmdLineOptionDescriptionChar{'C', CmdLineOptionType::CMDLOPT_NOARG, {}, "Read column names from the 1st line of file"}},
	{'B', CmdLineOptionDescriptionString{'B', CmdLineOptionType::CMDLOPT_STR, {"raise", "warn", "skip"}, "How to process bad lines"}},
	{'S', CmdLineOptionDescriptionNumber{'S', CmdLineOptionType::CMDLOPT_NUM, {}, "Skip first N lines of file"}}
};


namespace csviewer_internal
{
	void ThrowParseOptionException(const std::string& option_str, const std::string& message)
	{
		throw std::runtime_error("Error parsing command line option. " + message + ". Option: " + option_str);
	}

	bool IsTokenOption(const std::string& str, OptionKey& key_out)
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


	std::optional<CmdLineOptionDescriptionVariant> FindOptionByKey(const CmdLineOptionDescriptionContainer& options, OptionKey key)
	{
		auto it = options.find(key);
		if (it != options.end())
			return std::optional<CmdLineOptionDescriptionVariant>(it->second);
		return {};
	}


	bool IsOptionExistByKey(const CmdLineOptionDescriptionContainer& options, OptionKey key)
	{
		auto it = options.find(key);
		return it != options.end();
	}

	template <typename Var>
	bool IsOptionExistByKey(const std::vector<Var>& options, OptionKey key)
	{
		auto it = std::find_if(options.begin(), options.end(), [&key](const auto& option) {
			OptionKey option_key;
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


	std::optional<CmdLineOptionType> CmdLineOptionKeyToType(const CmdLineOptionDescriptionContainer& options, OptionKey key)
	{
		std::optional<CmdLineOptionType> type;

		if (auto opt = FindOptionByKey(options, key); opt)
		{
			auto type_getter = [&type](const auto& var) {
				type = var.type;
			};
			std::visit(type_getter, *opt);
			return type;
		}
		return {};
	}


	template<typename T>
	bool IsOptionValueAllowed(const CmdLineOptionDescriptionVariant& var, const T& value)
	{
		if (const auto obj(std::get_if<CmdLineOptionDescription<T>>(&var)); obj)
		{
			const auto& allowed = obj->allowed_values;
			if (allowed.empty())
				return true;
			auto result = std::find(allowed.begin(), allowed.end(), value);
			return (result != allowed.end()) ? true : false;
		}
		else
			return false;
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
	using namespace csviewer_internal;

	OptionKey key;

	if (!IsTokenOption(option_str, key))
		ThrowParseOptionException(option_str, "Defis not found");

	std::istringstream iss{ option_str };
	char defis;

	iss >> defis >> key;

	auto opt_type = CmdLineOptionKeyToType(g_ValidOptions, key);
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

	auto opt_it = FindOptionByKey(g_ValidOptions, key);

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
			if (IsOptionValueAllowed(*opt_it, c))
				return CmdLineOptionParseResultChar{ key, c };
			else
				ThrowParseOptionException(option_str, "Value not allowed: " + c);
		else
			ThrowParseOptionException(option_str, "Type: " + opt_type_str);
		break;
	}
	case CmdLineOptionType::CMDLOPT_STR:
	{
		std::string s;
		if (iss >> s)
			if (IsOptionValueAllowed(*opt_it, s))
				return CmdLineOptionParseResultString{ key, s };
			else
				ThrowParseOptionException(option_str, "Value not allowed: " + s);
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
	using namespace csviewer_internal;

	std::vector<CmdLineOptionParseResultVariant> parsed_options;
	std::string input_filename;

	try
	{
	if (argc < 2)
		throw std::runtime_error("Too few command line arguments.");

		bool is_found_input_filename = false;
		for (int i = 1; i < argc; i++)
		{
			std::string token_str(argv[i]);
			OptionKey key;
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
	}
	catch (std::runtime_error& ex)
	{
		throw std::runtime_error(std::string(ex.what()) + "\nPlease run csviewer -H for help.");
	}
	return CmdLineArgsParseResult{ parsed_options, input_filename};
 }

CSVLoadingSettings MakeSettingsByCmdLineArgs(const CmdLineArgsParseResult& cmd_line_args)
{
	CSVLoadingSettings settings(cmd_line_args.input_filename);

	static std::unordered_map<std::string, BadLinesPolicy> bad_line_policy_str2enum = {
		{"raise", BadLinesPolicy::BL_RAISE},
		{"warn", BadLinesPolicy::BL_WARN},
		{"skip", BadLinesPolicy::BL_SKIP}
	};

	// convert encoding string from cmd line arg to string that IConvConverter accepts
	auto encoding_str2settings = [](const auto& str) {
		std::string result;
#if(WIN32)
		std::transform(str.begin(), str.end(), std::back_inserter(result), ::std::toupper);
#else
		std::transform(str.begin(), str.end(), std::back_inserter(result), ::toupper);
#endif
		auto utf_index = result.find("UTF");
		if (utf_index != std::string::npos)
		{
			if (result[utf_index + 3] != '-')
				result.insert(utf_index + 3, "-");
		}
		return result;
	};

	for (const auto& option : cmd_line_args.options)
	{
		OptionKey option_key = std::visit([](const auto& option) { return option.key; }, option);

		// ugly "hardcode" impl of getting polymorphic value from variant
		std::optional<char> value_char;
		std::optional<std::string> value_string;
		std::optional<size_t> value_number;
		switch (option.index())
		{
		case 0: // char
		{
			auto value = std::get<0>(option).value;
			if (value.has_value())
				value_char.emplace(*value);
			break;
		}
		case 1: // std::string
		{
			auto value = std::get<1>(option).value;
			if (value.has_value())
				value_string.emplace(*value);
			break;
		}
		case 2: // size_t
		{
			auto value = std::get<2>(option).value;
			if (value.has_value())
				value_number.emplace(*value);
			break;
		}
		}

		switch (option_key)
		{
		case 'E':
			settings.encoding = encoding_str2settings(*value_string);
			break;
		case 'D':
			settings.delimiter = *value_char;
			break;
		case 'Q':
			settings.quote = *value_char;
			break;
		case 'C':
			settings.has_header = true;
			break;
		case 'B':
		{
			auto blp_iter = bad_line_policy_str2enum.find(*value_string);
			if (blp_iter != bad_line_policy_str2enum.end())
				settings.bad_lines_policy = blp_iter->second;
			break;
		}
		case 'S':
			settings.skip_first_lines = *value_number;
			break;
		default: break;
		}
	}
	return settings;
}

void PrintCmdLineHelp()
{
	std::cout << "CSViewer - console app for viewing CSV files." << std::endl;
	std::cout << "Usage: csviewer <file> [options]" << std::endl;
	std::cout << "Command line options: " << std::endl;
	for (const auto& [option_key, option_var] : g_ValidOptions)
		std::visit(CmdLineOptionDescriptionVariantHelpPrinter, option_var);
	std::cout << std::endl;
}