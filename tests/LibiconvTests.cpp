#include "gtest/gtest.h"
#include "IConvConverter.h"
#include <string.h>
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



TEST(LibiconvTests, Test1)
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