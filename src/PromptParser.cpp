#include "PromptParser.h"
#include <sstream>
#include <vector>
#include <iomanip>
#include "Command.h"


const char COMMAND_ARG_RANGE_DELIMITER = ':';
const char COMMAND_ARG_KEYVALUE_DELIMITER = '=';


namespace
{   /*
	bool IsFullyQuoted(const std::string& token)
	{
		static const char quote('"');
		if (token.size() < 2)
			return false;
		bool is_fully_quoted = (token.front() == quote) && (token.back() == quote);
		return (is_fully_quoted && (std::count(token.begin(), token.end(), quote) % 2 == 0));
	}
	*/
	std::string RemoveQuotes(const std::string& token)
	{
		std::stringstream ss{token};
		std::string result;
		ss >> std::quoted(result);
		return result;
	}

	bool IsFullyQuoted(const std::string& token, std::string& result)
	{
		result = RemoveQuotes(token);
		return result.size() == token.size() - 2;
	}

	bool ConvertToNumber(const std::string& token, int& result)
	{
		std::size_t pos = 0;
		try
		{
			const int i{ std::stoi(token, &pos) };
			if (pos != token.size())
				return false;
			result = i;
			return true;
		}
		catch (...)
		{
			return false;
		}
	}

	// left, right and both-sides string trimming in-place
	inline void LTrim(std::string& token)
	{
		token.erase(token.begin(), std::find_if(token.begin(), token.end(), [](auto ch) { return !std::isspace(ch); }));
	}
	// TODO: refactor to inline
	void RTrim(std::string& token)
	{
		static const char ws[] = " \t\n\r\f\v";
		token.erase(token.find_last_not_of(ws) + 1);
	}

	void Trim(std::string& token)
	{
		LTrim(token);
		RTrim(token);
	}

	// both-sides string trimming with copy
	std::string Trim(const std::string& token)
	{
		static const char ws[] = " \t\n\r\f\v";
		size_t left = token.find_first_not_of(ws);
		if (left == std::string::npos)
			return {};
		size_t right = token.find_last_not_of(ws, left);
		return token.substr(left, right - left + 1);
	}

	std::vector<std::string> Split(const std::string& token, char sep = ' ', bool remove_consecutive = false)
	{
		size_t old_pos = 0, pos = 0;
		std::vector<std::string> result;
		while (pos != std::string::npos && pos < token.size())
		{
			pos = token.find(sep, old_pos);
			if (!remove_consecutive && pos != old_pos)
				result.push_back(token.substr(old_pos, pos - old_pos));
			old_pos = pos + 1;
		}
		return result;
	}
	/*
	// TODO: alternative - variant_cast from https://stackoverflow.com/questions/47203255/convert-stdvariant-to-another-stdvariant-with-super-set-of-types
	template <typename ToVariant, typename FromVariant>
	ToVariant ConvertVariant(const FromVariant& v_from)
	{
		ToVariant v_to = std::visit([](FromVariant&& arg) -> ToVariant { return arg; }, v_from);
		return v_to;
	}
	*/

	template <class... Args>
	struct variant_cast_proxy
	{
		std::variant<Args...> v;

		template <class... ToArgs>
		operator std::variant<ToArgs...>() const
		{
			return std::visit([](auto&& arg) -> std::variant<ToArgs...> { return arg; },
				v);
		}
	};

	template <class... Args>
	auto variant_cast(const std::variant<Args...>& v) -> variant_cast_proxy<Args...>
	{
		return { v };
	}
}

std::optional<CommandArgPrimitiveType> ParseTokenToPrimitiveType(const std::string& token)
{
	static const char disallowed_chars[] = { ' ', '\t', '\n', '\r', '\f', '\v', COMMAND_ARG_RANGE_DELIMITER, COMMAND_ARG_KEYVALUE_DELIMITER };
	std::string unquoted_token;
	int number;
	if (IsFullyQuoted(token, unquoted_token))
	{
		return CommandArgString(unquoted_token);
	}
	else if (ConvertToNumber(token, number))
	{
		return CommandArgNumber(number);
	}
	else if (token.find_last_of(disallowed_chars) == std::string::npos)
	{
		return CommandArgString(token);
	}
	return std::nullopt;
}

CommandArgVariant ParseToken(const std::string& token)
{
	auto primitive_type = ParseTokenToPrimitiveType(token);
	if (primitive_type)
		return variant_cast(*primitive_type);

	if (std::count(token.begin(), token.end(), COMMAND_ARG_RANGE_DELIMITER) == 1)
	{
		auto operands_tokens = Split(token, COMMAND_ARG_RANGE_DELIMITER);
		if (operands_tokens.size() != 2 || operands_tokens[0].empty() || operands_tokens[1].empty())
			throw std::logic_error("Bad range definition: " + token);

		auto left_op = ParseTokenToPrimitiveType(operands_tokens[0]);
		auto right_op = ParseTokenToPrimitiveType(operands_tokens[1]);
		if (left_op && right_op)
		{
			auto left_op_str = std::get_if<CommandArgString>(&*left_op);
			auto right_op_str = std::get_if<CommandArgString>(&*right_op);
			if (left_op_str && right_op_str)
				return CommandArgStringRange(*left_op_str, *right_op_str);

			auto left_op_number = std::get_if<CommandArgNumber>(&*left_op);
			auto right_op_number = std::get_if<CommandArgNumber>(&*right_op);
			if (left_op_number && right_op_number)
				return CommandArgNumberRange(*left_op_number, *right_op_number);
			else
				throw std::logic_error("Bad operand types for range definition: " + token);
		}
		else
			throw std::logic_error("Bad range definition: " + token);;
	}
	else if (std::count(token.begin(), token.end(), COMMAND_ARG_KEYVALUE_DELIMITER) == 1)
	{
		auto operands_tokens = Split(token, COMMAND_ARG_KEYVALUE_DELIMITER);
		if (operands_tokens.size() != 2 || operands_tokens[0].empty() || operands_tokens[1].empty())
			throw std::logic_error("Bad key-value definition: " + token);

		auto left_op = ParseTokenToPrimitiveType(operands_tokens[0]);
		auto right_op = ParseTokenToPrimitiveType(operands_tokens[1]);
		if (left_op && right_op)
			return CommandArgKeyValuePair(*left_op, *right_op);
		else
			throw std::logic_error("Bad range definition: " + token);
	}
	else
		throw std::logic_error("Failed to parse token: " + token);
}


CommandParseResult ParsePromptInput(const std::string& prompt_input)
{
	CommandParseResult result;
	std::stringstream ss(prompt_input);
	std::string command_token;
	std::vector<std::string> args_tokens;
	ss >> command_token;

	// parse arguments here
	if (command_token == "head" || command_token == "h")
	{
		std::string token;
		ss >> token;
	}
	else
	{
		throw std::logic_error("unknown command");
	}

	result.command = command_token;
	return result;
}