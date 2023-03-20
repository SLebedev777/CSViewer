#include "gtest/gtest.h"
#include "IConvConverter.h"
#include <string>
#include <vector>

namespace
{
    std::string Win1251ToUtf8(const std::string& a_Win1251Str)
    {
        try
        {
            auto conv = utils::IConvConverter("CP1251", "UTF-8");
            std::string result = conv.convert(a_Win1251Str);
            return result;
        }
        catch (const std::runtime_error&)
        {
            return {};
        }
    }
}

std::string Utf8ToCp1251(const std::string& a_Utf8Str, utils::IConvConverter::EncodingErrorMode error_mode)
{
    try
    {
        auto conv = utils::IConvConverter("UTF-8", "CP1251");
        std::string result = conv.convert(a_Utf8Str, error_mode);
        return result;
    }
    catch (const std::runtime_error&)
    {
        return {};
    }
}



TEST(LibiconvTests, Cp1251ToUtf8Test1)
{
    // ARRANGE
    const std::string win1251_original_str = "банан";
    const std::string utf8_expected_output_str = "\xD0\xB1\xD0\xB0\xD0\xBD\xD0\xB0\xD0\xBD";

    ASSERT_NE(win1251_original_str, utf8_expected_output_str);

    //ACT
    std::string result = Win1251ToUtf8(win1251_original_str);

    //ASSERT
    EXPECT_EQ(utf8_expected_output_str, result);

}

TEST(LibiconvTests, Cp1251ToUtf8Test2)
{
    std::string result = Win1251ToUtf8("banana-mama нан %$&*^№");
    EXPECT_EQ("banana-mama \xD0\xBD\xD0\xB0\xD0\xBD %$&*^\xE2\x84\x96", result);
}

TEST(Utf82Win1251ConvertionTest, Utf8ToCp1251_InvalidSymbolTest)
{
    using iconv = utils::IConvConverter;

    // Оригинальная строчка - "start_{иероглиф}_finish"
    const std::string utf8_original_str = "start_\xE3\x82\x92_finish";

    std::string result = Utf8ToCp1251(utf8_original_str, iconv::EncodingErrorMode::REPLACE);
    EXPECT_EQ("start_???_finish", result);
    std::string result2 = Utf8ToCp1251(utf8_original_str, iconv::EncodingErrorMode::IGNORE);
    EXPECT_EQ("start__finish", result2);

    auto conv = utils::IConvConverter("UTF-8", "CP1251");
    EXPECT_THROW(conv.convert(utf8_original_str, iconv::EncodingErrorMode::STRICT), std::runtime_error);
}
