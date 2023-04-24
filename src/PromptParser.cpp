#include "PromptParser.h"
#include "StringUtils.h"
#include <sstream>
#include <vector>
#include <iomanip>
#include <list>
#include <numeric>


const char COMMAND_ARG_RANGE_DELIMITER = ':';
const char COMMAND_ARG_KEYVALUE_DELIMITER = '=';
const char COMMAND_KEYWORD_DELIMITER = '|';
const char COMMAND_ARGS_DELIMITER = ',';

const size_t NO_ARGS = 0;
const size_t UNLIMITED_ARGS = std::numeric_limits<size_t>::max();


// словарь доступных опций, поддерживаемых программой
const std::vector<CommandSyntaxDescription> g_ValidPromptCommands = {
	{"head", "h",
		{
			{
				{ {}, false, {CommandArgNumber{}}, NO_ARGS, 1 }
			}
		}
	},
	{"tail", "t",
		{
			{
				{ {}, false, {CommandArgNumber{}}, NO_ARGS, 1 }
			}
		}
	},
	{"view", "v",
		{
			{
				{ {}, false, {CommandArgKeyValuePair{}}, 1, UNLIMITED_ARGS }
			}
		}
	},
	{"cols", "c",
		{
			{
				{ {}, false, {}, NO_ARGS, NO_ARGS }
			},
			{
				{ {}, false, {CommandArgKeyValuePair{}}, 1, UNLIMITED_ARGS }
			},
			{
				{ {}, false, {CommandArgString{}}, 1, UNLIMITED_ARGS }
			},
			{
				{ {"header"}, true, {}, NO_ARGS, NO_ARGS}
			}
		}
	},
	{"shape", "s",
		{
			{
				{ {}, false, {}, NO_ARGS, NO_ARGS }
			}
		}
	},
	{"quit", "q",
		{
			{
				{ {}, false, {}, NO_ARGS, NO_ARGS }
			}
		}
	},
	{"print", "p",
		{
			{   // p [row] [R1, <M1:N1>, <M2:N2>, R2, ...]
				{ {"row"}, false,
					{
						CommandArgNumber{},
						CommandArgNumberRange{}
					}, NO_ARGS, UNLIMITED_ARGS
				}
			},
			{   // p col R1, <M1:N1>, <M2:N2>, R2, ...
				{ {"col"}, true,
					{
						CommandArgNumber{},
						CommandArgString{},
						CommandArgNumberRange{},
						CommandArgStringRange{}
					}, 1, UNLIMITED_ARGS
				}
			},
			{   // p [row] R1, <M1:N1>, <M2:N2>, R2, ... | col R1, <M1:N1>, <M2:N2>, R2, ... 
				{ {"row"}, false,
					{
						CommandArgNumber{},
						CommandArgNumberRange{}
					}, 1, UNLIMITED_ARGS
				},
				{ {"col"}, true,
					{
						CommandArgNumber{},
						CommandArgString{},
						CommandArgNumberRange{},
						CommandArgStringRange{}
					}, 1, UNLIMITED_ARGS
				},
			}

		}
	}

};

namespace
{
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

	bool IsKeywordAllowed(const std::string& kw, const CommandSyntaxDescription& command_syntax_descr)
	{
		for (auto& syntax_var : command_syntax_descr.keywords_and_args)
		{
			for (auto& kw_syntax_descr : syntax_var)
			{
				auto it = std::find(kw_syntax_descr.allowed_kw_values.begin(), kw_syntax_descr.allowed_kw_values.end(), kw);
				if (it == kw_syntax_descr.allowed_kw_values.end())
					return false;
			}
		}
		return true;
	}

	bool FilterMatchingSyntaxVariants(const CommandSyntaxDescription& command_syntax_descr, 
		std::list<size_t>& matching_syntax_variants_indices,  /* OUT */
		size_t curr_kw_index, 
		const std::string& kw)
	{
		bool kw_detected = false;

		matching_syntax_variants_indices.remove_if([&](auto i) {
			if (curr_kw_index >= command_syntax_descr.keywords_and_args[i].size())
				return true;

			const auto& allowed_kw_values = command_syntax_descr.keywords_and_args[i][curr_kw_index].allowed_kw_values;
			auto it = std::find(allowed_kw_values.begin(), allowed_kw_values.end(), kw);
			// ключевого слова нет среди разрешенных в данном варианте синтаксиса.
			if (it == allowed_kw_values.end())
			{
				return command_syntax_descr.keywords_and_args[i][curr_kw_index].is_kw_required;
			}
			kw_detected = true;
			return false;  // не удалять этот вариант синтаксиса
			});

		if (std::any_of(matching_syntax_variants_indices.begin(), matching_syntax_variants_indices.end(), [&](auto i) {
			return command_syntax_descr.keywords_and_args[i][curr_kw_index].is_kw_required;
			}))
		{
			matching_syntax_variants_indices.remove_if([&](auto i) {
				return command_syntax_descr.keywords_and_args[i][curr_kw_index].is_kw_required == false;
				});
		}

			return kw_detected;
	}

	void FilterMatchingSyntaxVariants(const CommandSyntaxDescription& command_syntax_descr,
		std::list<size_t>& matching_syntax_variants_indices,  /* OUT */
		size_t curr_kw_index,
		const CommandArgVariant& arg)
	{
		matching_syntax_variants_indices.remove_if([&](auto i) {
			if (curr_kw_index >= command_syntax_descr.keywords_and_args[i].size())
				return true;

			const auto& allowed_args_types = command_syntax_descr.keywords_and_args[i][curr_kw_index].allowed_args_types;
			return std::none_of(allowed_args_types.begin(), allowed_args_types.end(),
				[&arg](auto item) {
					return arg.index() == item.index();
				});
			});
	}

	void FilterMatchingSyntaxVariants(const CommandSyntaxDescription& command_syntax_descr,
		std::list<size_t>& matching_syntax_variants_indices,  /* OUT */
		size_t curr_kw_index,
		size_t num_args)
	{
		matching_syntax_variants_indices.remove_if([&](auto i) {
			if (curr_kw_index >= command_syntax_descr.keywords_and_args[i].size())
				return true;

			const auto& kw_syntax = command_syntax_descr.keywords_and_args[i][curr_kw_index];
			return num_args < kw_syntax.min_args || num_args > kw_syntax.max_args;
			});
	}

}


std::optional<CommandArgPrimitiveType> ParseTokenToPrimitiveType(const std::string& token)
{
	static const char disallowed_chars[] = { ' ', '\t', '\n', '\r', '\f', '\v', COMMAND_ARG_RANGE_DELIMITER, COMMAND_ARG_KEYVALUE_DELIMITER, '\0'};
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
			throw PromptParserException("Bad range definition: " + token);

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
				throw PromptParserException("Bad operand types for range definition: " + token);
		}
		else
			throw PromptParserException("Bad range definition: " + token);;
	}
	else if (std::count(token.begin(), token.end(), COMMAND_ARG_KEYVALUE_DELIMITER) == 1)
	{
		auto operands_tokens = Split(token, COMMAND_ARG_KEYVALUE_DELIMITER);
		if (operands_tokens.size() != 2 || operands_tokens[0].empty() || operands_tokens[1].empty())
			throw PromptParserException("Bad key-value definition: " + token);

		auto left_op = ParseTokenToPrimitiveType(operands_tokens[0]);
		auto right_op = ParseTokenToPrimitiveType(operands_tokens[1]);
		if (left_op && right_op)
			return CommandArgKeyValuePair(*left_op, *right_op);
		else
			throw PromptParserException("Bad range definition: " + token);
	}
	else
		throw PromptParserException("Failed to parse token: " + token);
}


bool operator==(const CommandKeywordSyntaxDescription& left, const CommandKeywordSyntaxDescription& right)
{
	return left.is_kw_required == right.is_kw_required &&
		left.min_args == right.min_args &&
		left.max_args == right.max_args &&
		left.allowed_args_types == right.allowed_args_types &&
		left.allowed_kw_values == right.allowed_kw_values;
}

bool operator==(const CommandSyntaxDescription& left, const CommandSyntaxDescription& right)
{
	return left.command_full == right.command_full &&
		right.command_short == right.command_short &&
		left.keywords_and_args == right.keywords_and_args;
}


CommandParseResult ParsePromptInput(const std::string& prompt_input)
{
	CommandParseResult result;
	std::stringstream ss(prompt_input);
	std::string command_token;
	ss >> command_token;

	if (command_token.empty())
		throw PromptParserException("ParsePromptInput error: empty command");

	// get valid command descriptions for known command token
	auto valid_command_it = std::find_if(g_ValidPromptCommands.begin(), g_ValidPromptCommands.end(), [&](const auto& item) {
		return (item.command_full == command_token) ||
			   (item.command_short == command_token);
		});
	if (valid_command_it == g_ValidPromptCommands.end())
	{
		throw PromptParserException("ParsePromptInput error: unknown command: " + command_token);
	}

	CommandSyntaxDescription command_syntax_descr = *valid_command_it;
	result.command = command_syntax_descr.command_full;

	// parse arguments here
	
	// алгоритм:
	// считать ключевое слово
	// если не подходит - выход с ошибкой
	// найти конец списка аргументов - это символ | (если нет - конец строки)
	// разделить аргументы через запятую
	// по очереди считывать каждый аргумент
	//  если не подходит по типу - выход с ошибкой

	// TODO: сейчас для простоты делаем все ключевые слова обязательными. Сделать опциональные ключевые слова потом!
	
	// Список индексов вариантов синтаксиса команды, которые в данный момент подходят под разбираемую строку ввода.
	// В начале разбора команды, все варианты синтаксиса считаем подходящими.
	// Но по мере того, как парсим ключевые слова и аргументы, набор подходящих вариантов синтаксиса будет просеиваться
	// (неподходящие варианты будут удаляться из списка).
	// Если список индексов стал пуст - ни один вариант не подходит - то значит строка ввода не валидна, и не может быть превращена
	// в одну из поддерживаемых программой команд.
	// По идее, в результате успешного разбора строки ввода, по окончании парсинга, должен остаться лишь 1 какой-то вариант синтаксиса команды.
	std::list<size_t> matching_syntax_variants_indices(command_syntax_descr.keywords_and_args.size(), 0);
	std::iota(matching_syntax_variants_indices.begin(), matching_syntax_variants_indices.end(), 0);
	
	// Сюда будет писаться индекс текущего разбираемого ключевого слова для каждого варианта синтаксиса.
	// Когда для данного варианта синтаксиса, какое-то ключевое слово вместе со всеми своими аргументами успешно разобрано,
	// надо перейти к следующему ключевому слову - тут мы увеличиваем текущий индекс ключевого слова.	
	size_t kw_index = 0;

	while (!ss.eof())
	{
		KeywordAndArgs kw_args;

		// логика парсинга первого ключевого слова - которое для каких-то вариантов синтаксиса может быть не обязательным:
		// 
		// 1) запоминаем позицию потока ss
		// 2) считать первый токен после токена команды с разделителем "пробел" - kw
		// 3) для каждого варианта синтаксиса команды проводим первичную фильтрацию:
		//		- если значение kw входит в список допустимых ключ. слов - считаем, что именно это ключ. слово и было введено. Вариант синтаксиса считаем подходящим.
		//		- иначе:
		//			- если признак is_kw_required == True - вариант синтаксиса исключаем
		//			- если признак is_kw_required == False: - вариант синтаксиса считаем (пока) подходящим.
		// 4) Если после первичной фильтрации есть хотя бы 1 подходящий вариант синтаксиса с is_kw_required == True,
		//		все имеющиеся варианты с is_kw_required == False исключаем.
		// 5) Итого, после фильтрации вариантов синтаксиса:
		//		- если остались только варианты синтаксиса с is_kw_required == True:
		//				- ключевое слово kw успешно распарсено. Перейти к чтению аргументов из потока ss.
		//		- иначе (остались только варианты синтаксиса с is_kw_required == False):
		//				- вернуться в потоке ss на позицию до ключевого слова (из п.1)
		//				- Перейти к чтению аргументов из ss.

		std::stringstream::pos_type kw_pos = ss.tellg();
		std::string kw;
		ss >> kw;

		bool kw_detected = FilterMatchingSyntaxVariants(command_syntax_descr, matching_syntax_variants_indices, kw_index, kw);
		if (matching_syntax_variants_indices.empty())
			throw PromptParserException("ParsePromptInput error: unknown keyword: " + kw);
		
		if (kw_detected)
		{
			kw_args.kw = kw;
		}
		else
		{
			ss.seekg(kw_pos);
		}

		std::string args_str;
		std::getline(ss >> std::ws, args_str, COMMAND_KEYWORD_DELIMITER);
		std::stringstream args_ss{ args_str };
		size_t num_args = 0;
		for (std::string token; std::getline(args_ss >> std::ws, token, COMMAND_ARGS_DELIMITER); num_args++)
		{
			Trim(token);
			if (num_args > 0 && token.empty())
				throw PromptParserException("ParsePromptInput error: keyword = " + kw_args.kw + ", empty non-first argument");

			auto arg = ParseToken(token);
			FilterMatchingSyntaxVariants(command_syntax_descr, matching_syntax_variants_indices, kw_index, arg);
			if (matching_syntax_variants_indices.empty())
				throw PromptParserException("ParsePromptInput error: keyword = " + kw_args.kw + ", arg type not allowed: " + token);

			kw_args.args.push_back(arg);
		}

		FilterMatchingSyntaxVariants(command_syntax_descr, matching_syntax_variants_indices, kw_index, num_args);
		if (matching_syntax_variants_indices.empty())
			throw PromptParserException("ParsePromptInput error: keyword = " + kw_args.kw + ", wrong number of arguments: " + std::to_string(num_args));

		result.keywords_and_args.push_back(std::move(kw_args));
		++kw_index;
	}
	return result;
}

bool operator==(const KeywordAndArgs& left, const KeywordAndArgs& right)
{
	return (left.kw == right.kw) && (left.args == right.args);
}

bool operator==(const CommandParseResult& left, const CommandParseResult& right)
{
	return (left.command == right.command) && (left.keywords_and_args == right.keywords_and_args);
}

std::ostream& operator<<(std::ostream& oss, const CommandParseResult& cpr)
{
	oss << "Command: " << cpr.command << std::endl;
	for (auto kw_args : cpr.keywords_and_args)
	{
		oss << "\tKeyword: " << kw_args.kw << ", num_args: " << kw_args.args.size() << std::endl;
	}
	return oss;
}