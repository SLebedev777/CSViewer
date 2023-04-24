#include "gtest/gtest.h"
#include "PromptParser.h"
#include <numeric>

namespace
{
	auto unquoted = [](const std::string& s) { return s.substr(1, s.size() - 2); };
}


TEST(PromptParserTests, ParseTokenToQuotedString)
{
	{
		std::string token("\"Data\"");
		auto result = ParseToken(token);

		EXPECT_EQ(std::get<CommandArgString>(result), token.substr(1, token.size() - 2));
	}
	{
		std::string token("\"Data with spaces!\"");
		auto result = ParseToken(token);

		EXPECT_EQ(std::get<CommandArgString>(result), token.substr(1, token.size() - 2));
	}
	{
		std::string token("\"123\"");
		auto result = ParseToken(token);

		EXPECT_EQ(std::get<CommandArgString>(result), token.substr(1, token.size() - 2));
	}
	{
		std::string token("\"123 qwe	!@#\"");
		auto result = ParseToken(token);

		EXPECT_EQ(std::get<CommandArgString>(result), token.substr(1, token.size() - 2));
	}
	{
		std::string token("\"Range:Example\"");
		auto result = ParseToken(token);

		EXPECT_EQ(std::get<CommandArgString>(result), token.substr(1, token.size() - 2));
	}
	{
		std::string token("\"Data\" not fully quoted");

		EXPECT_THROW(ParseToken(token), PromptParserException);
	}
	{
		std::string token("\"Data not fully quoted");

		EXPECT_THROW(ParseToken(token), PromptParserException);
	}
	{
		std::string token("\"Many\" \"quotes\"");

		EXPECT_THROW(ParseToken(token), PromptParserException);
	}

}

TEST(PromptParserTests, ParseTokenToNumber)
{
	{
		std::string token("123");
		auto result = ParseToken(token);

		EXPECT_EQ(std::get<CommandArgNumber>(result), std::stoi(token));
	}
	{
		constexpr int max_int = std::numeric_limits<int>::max();
		std::string token(std::to_string(max_int));
		auto result = ParseToken(token);

		EXPECT_EQ(std::get<CommandArgNumber>(result), max_int);
	}
	{
		std::string token("-12345");
		auto result = ParseToken(token);

		EXPECT_EQ(std::get<CommandArgNumber>(result), std::stoi(token));
	}
	{
		std::string token("123text");
		auto result = ParseToken(token);

		EXPECT_EQ(std::get_if<CommandArgNumber>(&result), nullptr);
	}
	{
		std::string token("3.1415");
		auto result = ParseToken(token);

		EXPECT_EQ(std::get_if<CommandArgNumber>(&result), nullptr);
	}
}

TEST(PromptParserTests, ParseTokenToString)
{
	{
		std::string token("JustAString");
		auto result = ParseToken(token);

		EXPECT_EQ(std::get<CommandArgString>(result), token);
	}
}

TEST(PromptParserTests, ParseTokenToStringRange)
{
	{
		std::string left = "From", right = "To";
		std::string token(left + ":" + right);
		auto result = ParseToken(token);

		auto expected = CommandArgStringRange{ left, right };
		EXPECT_EQ(std::get<CommandArgStringRange>(result), expected);
	}
	{
		std::string left = "\"From 123 with spaces\"", right = "\"To 456 with spaces\"";
		std::string token(left + ":" + right);
		auto result = ParseToken(token);

		auto expected = CommandArgStringRange{ unquoted(left), unquoted(right) };
		EXPECT_EQ(std::get<CommandArgStringRange>(result), expected);
	}
}

TEST(PromptParserTests, ParseTokenToNumberRange)
{
	{
		int left = 123, right = 456;
		std::string token(std::to_string(left) + ":" + std::to_string(right));
		auto result = ParseToken(token);

		auto expected = CommandArgNumberRange{ left, right };
		EXPECT_EQ(std::get<CommandArgNumberRange>(result), expected);
	}
	{
		int left = -123, right = 0;
		std::string token(std::to_string(left) + ":" + std::to_string(right));
		auto result = ParseToken(token);

		auto expected = CommandArgNumberRange{ left, right };
		EXPECT_EQ(std::get<CommandArgNumberRange>(result), expected);
	}
	{
		std::string token("5:Text");
		EXPECT_THROW(ParseToken(token), PromptParserException);
	}
	{
		std::string token("Many:Colons:In:Token");
		EXPECT_THROW(ParseToken(token), PromptParserException);
	}

}

TEST(PromptParserTests, ParseTokenToKeyValuePair)
{
	{
		CommandArgNumber key = 123, value = 456;
		std::string token(std::to_string(key) + "=" + std::to_string(value));
		auto result = ParseToken(token);

		auto expected = CommandArgKeyValuePair{ key, value };
		EXPECT_EQ(std::get<CommandArgKeyValuePair>(result), expected);
	}
	{
		CommandArgString key = "\"Quoted Key\"";
		CommandArgNumber value = 456;
		std::string token(key + "=" + std::to_string(value));
		auto result = ParseToken(token);

		auto expected = CommandArgKeyValuePair{ unquoted(key), value };
		EXPECT_EQ(std::get<CommandArgKeyValuePair>(result), expected);
	}
	{
		CommandArgString key = "\"Quoted Key\"";
		CommandArgString value = "\"Quoted Value\"";
		std::string token(key + "=" + value);
		auto result = ParseToken(token);

		auto expected = CommandArgKeyValuePair{ unquoted(key), unquoted(value) };
		EXPECT_EQ(std::get<CommandArgKeyValuePair>(result), expected);
	}

}

TEST(PromptParserTests, ParsePromptInputPrintRowGood)
{
	std::string input{ "p row 1, 2, 505:888, 999" };
	
	CommandParseResult expected {
		"print",
		{
			{"row", {CommandArgNumber{1}, CommandArgNumber{2}, CommandArgNumberRange(505, 888), CommandArgNumber{999}}}
		}
	};

	auto result = ParsePromptInput(input);

	EXPECT_EQ(expected, result);
}

TEST(PromptParserTests, ParsePromptInputPrintRowBadArgType)
{
	std::string input{ "p row 1, 2, A:B" };

	EXPECT_THROW(ParsePromptInput(input), PromptParserException);
}


TEST(PromptParserTests, ParsePromptInputPrintColGood)
{
	std::string input{ "p col 1, \"Column One\",A:B, \"Col 1\":\"Col 2\",   5:7" };

	CommandParseResult expected{
		"print",
		{
			{"col", {CommandArgNumber{1},
					 CommandArgString{"Column One"},
					 CommandArgStringRange("A", "B"),
					 CommandArgStringRange("Col 1", "Col 2"),
					 CommandArgNumberRange(5, 7)
					}
			}
		}
	};

	auto result = ParsePromptInput(input);

	EXPECT_EQ(expected, result);
}

TEST(PromptParserTests, ParsePromptInputPrintRowOptionalKeyword)
{
	std::string input{ "p 1, 2, 505:888, 999" };

	CommandParseResult expected{
		"print",
		{
			{"", {CommandArgNumber{1}, CommandArgNumber{2}, CommandArgNumberRange(505, 888), CommandArgNumber{999}}}
		}
	};

	auto result = ParsePromptInput(input);

	EXPECT_EQ(expected, result);
}

TEST(PromptParserTests, ParsePromptInputPrintRowOptionalKeywordBadArgType)
{
	std::string input{ "p 1, 2, A:B" };

	EXPECT_THROW(ParsePromptInput(input), PromptParserException);
}

TEST(PromptParserTests, ParsePromptInputHead)
{
	std::string input{ "h 10" };

	CommandParseResult expected{
		"head",
		{
			{"", {CommandArgNumber{10}}}
		}
	};

	auto result = ParsePromptInput(input);

	EXPECT_EQ(expected, result);
}

TEST(PromptParserTests, ParsePromptInputHeadWrongNumberOfArgs)
{
	std::string input{ "h 10, 20, 30" }; // none or one arg allowed, not more

	EXPECT_THROW(ParsePromptInput(input), PromptParserException);
}

TEST(PromptParserTests, ParsePromptInputPrintColWrongNumberOfArgs)
{
	std::string input{ "print col" }; // at least 1 arg required

	EXPECT_THROW(ParsePromptInput(input), PromptParserException);
}

TEST(PromptParserTests, ParsePromptInputNoKwNoArgs)
{
	{
		std::string input{ "h" };

		CommandParseResult expected{
			"head",
			{
			}
		};

		auto result = ParsePromptInput(input);

		EXPECT_EQ(expected, result);
	}
	{
		std::string input{ "p" };

		CommandParseResult expected{
			"print",
			{
			}
		};

		auto result = ParsePromptInput(input);

		EXPECT_EQ(expected, result);
	}

}