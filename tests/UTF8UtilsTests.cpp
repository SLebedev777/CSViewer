#include "gtest/gtest.h"
#include <string>
#include "UTF8Utils.h"


TEST(UTF8UtilsTests, UTF8StrLenTest1)
{
    // ARRANGE
    const std::string utf8_str = "\xD0\xB1\xD0\xB0\xD0\xBD\xD0\xB0\xD0\xBD"; // банан

    //ASSERT
    EXPECT_EQ(5, Utf8StrLen(utf8_str));
}

TEST(UTF8UtilsTests, UTF8StrLenTest2)
{
    const std::string utf8_str = "banana-mama \xD0\xBD\xD0\xB0\xD0\xBD %$&*^\xE2\x84\x96"; // banana-mama нан %$&*^№

    EXPECT_EQ(22, Utf8StrLen(utf8_str));
}

TEST(UTF8UtilsTests, UTF8SubStrTest1)
{
    // ARRANGE
    const std::string utf8_str = "banana-mama \xD0\xBD\xD0\xB0\xD0\xBD %$&*^\xE2\x84\x96"; // banana-mama нан %$&*^№

    // EXPECT
    std::string result = "banana-mama \xD0\xBD"; // banana-mama н

    EXPECT_EQ(result, Utf8SubStr(utf8_str, 0, 13));
}

TEST(UTF8UtilsTests, UTF8SubStrTest2)
{
    // ARRANGE
    const std::string utf8_str = "banana-mama"; // banana-mama

    EXPECT_EQ(utf8_str.substr(7, 3), Utf8SubStr(utf8_str, 7, 3));
}

TEST(UTF8UtilsTests, UTF8SubStrTest3)
{
    // ARRANGE
    const std::string utf8_str = "\xD0\xB1\xD0\xB0\xD0\xBD\xD0\xB0\xD0\xBD mama %$&*^\xE2\x84\x96 \xD0\xBD\xD0\xB0\xD0\xBD"; // банан mama %$&*^№ нан

    // EXPECT
    std::string result = "%$&*^\xE2\x84\x96 \xD0\xBD"; // %$&*^№ н

    EXPECT_EQ(result, Utf8SubStr(utf8_str, 11, 8));
}
