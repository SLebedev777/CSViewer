#include "gtest/gtest.h"
#include "../externals/libiconv/include/iconv.h"
#include <string.h>
#include <vector>

namespace
{
    std::string Win1251ToUtf8(const std::string& a_Win1251Str)
    {
        try
        {
            iconv_t conv = ::iconv_open("UTF-8", "CP1251");
            if (conv == (iconv_t)-1)
            {
                throw std::runtime_error("libiconv error");
            }

            size_t BUFSIZE = 1024;
            std::vector<char> buf(BUFSIZE); // future result
            char* dst_ptr = &buf[0];  // should NOT be const, because libiconv will write through it
            size_t dst_size = BUFSIZE; // here libiconv will write actual size of converted result string
            const char* src_ptr = a_Win1251Str.data();
            size_t SRCSIZE = a_Win1251Str.size();
            size_t res = ::iconv(conv, &src_ptr, &SRCSIZE, &dst_ptr, &dst_size);
            std::string result;
            result.append(&buf[0], BUFSIZE - dst_size);

            iconv_close(conv);

            return result;
        }
        catch (const std::runtime_error&)
        {
            // Error handling...
            return std::string();
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
    const std::string result = Win1251ToUtf8(win1251_original_str);

    //ASSERT
    EXPECT_EQ(utf8_expected_output_str, result);

}