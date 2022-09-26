#include "gtest/gtest.h"
#include "CmdLineParser.h"


TEST(CmdLineParserTests, IsTokenOption)
{
    using namespace csviewer_internal;

    char key;

    EXPECT_TRUE(IsTokenOption("-A", key));
    EXPECT_FALSE(IsTokenOption("-a", key));
    EXPECT_TRUE(IsTokenOption("-A1", key));
    EXPECT_TRUE(IsTokenOption("-Aq", key));
    EXPECT_FALSE(IsTokenOption("-aq", key));
    EXPECT_TRUE(IsTokenOption("-Aqwerty", key));
    EXPECT_TRUE(IsTokenOption("-AAAA", key));
    EXPECT_TRUE(IsTokenOption("-AA1", key));
    EXPECT_FALSE(IsTokenOption("-aAA", key));
    EXPECT_TRUE(IsTokenOption("-Aqwe asd", key));  // actual value is not parsed here, only that it's token
    EXPECT_FALSE(IsTokenOption("-!", key));
    EXPECT_FALSE(IsTokenOption("-1", key));
    EXPECT_FALSE(IsTokenOption("-,abc", key));
    EXPECT_FALSE(IsTokenOption("-!", key));
    EXPECT_FALSE(IsTokenOption("-", key));
    EXPECT_FALSE(IsTokenOption("~A", key));
    EXPECT_FALSE(IsTokenOption("A-", key));
}


TEST(CmdLineParserTests, DetectInputFilenameIsPresent) {
    // ARRANGE
    char* argv[] = {"csviewer.exe", "test.csv"};
    int argc = 2;
    
    // ACT
    auto result = ParseCmdLineArgs(argc, argv);

    // EXPECT
    CmdLineArgsParseResult expected;
    expected.input_filename = "test.csv";
    EXPECT_EQ(expected.input_filename, result.input_filename);
}


TEST(CmdLineParserTests, DetectInputFilenameIsAbsent) {
    // ARRANGE
    char* argv[] = { "csviewer.exe", "-C" };
    int argc = 2;

    // ACT

    // EXPECT
    EXPECT_ANY_THROW(ParseCmdLineArgs(argc, argv));
}


TEST(CmdLineParserTests, DetectHelpOptionWithoutInputFilename) {
    // ARRANGE
    char* argv[] = { "csviewer.exe", "-H" };
    int argc = 2;

    // ACT
    auto result = ParseCmdLineArgs(argc, argv);

    // EXPECT
    std::vector<CmdLineOptionParseResultVariant> expected_options = {
        CmdLineOptionParseResultChar{ 'H', {} }
    };
    CmdLineArgsParseResult expected{ expected_options, "" };
    EXPECT_EQ(result, expected);
}


TEST(CmdLineParserTests, DetectHelpOptionWithInputFilename) {
    // ARRANGE
    char* argv[] = { "csviewer.exe", "-H", "test.csv" };
    int argc = 3;

    // ACT
    auto result = ParseCmdLineArgs(argc, argv);

    // EXPECT
    std::vector<CmdLineOptionParseResultVariant> expected_options = {
        CmdLineOptionParseResultChar{ 'H', {} }
    };
    CmdLineArgsParseResult expected{expected_options, "test.csv"};
    EXPECT_EQ(result, expected);
}

TEST(CmdLineParserTests, StringOptionWithAllowedValue) {
    // ARRANGE
    char* argv[] = { "csviewer.exe", "-Eutf8", "test.csv" };
    int argc = 3;

    // ACT
    auto result = ParseCmdLineArgs(argc, argv);

    // EXPECT
    std::vector<CmdLineOptionParseResultVariant> expected_options = {
        CmdLineOptionParseResultString{ 'E', "utf8"}
    };
    CmdLineArgsParseResult expected{ expected_options, "test.csv" };
    EXPECT_EQ(result, expected);
}


TEST(CmdLineParserTests, StringOptionWithDisallowedValue) {
    // ARRANGE
    char* argv[] = { "csviewer.exe", "-Ewrong", "test.csv" };
    int argc = 3;

    EXPECT_ANY_THROW(ParseCmdLineArgs(argc, argv));
}


TEST(CmdLineParserTests, CharOptionWithAllowedValue2) {
    // ARRANGE
    char* argv[] = { "csviewer.exe", "-D!", "test.csv"};
    int argc = 3;

    // ACT
    auto result = ParseCmdLineArgs(argc, argv);

    // EXPECT
    std::vector<CmdLineOptionParseResultVariant> expected_options = {
        CmdLineOptionParseResultChar{ 'D', '!'}
    };
    CmdLineArgsParseResult expected{ expected_options, "test.csv" };
    EXPECT_EQ(result, expected);

}


TEST(CmdLineParserTests, StringOptionWithDisallowedValue2) {
    // ARRANGE
    char* argv[] = { "csviewer.exe", "-Q2", "test.csv" };
    int argc = 3;

    EXPECT_ANY_THROW(ParseCmdLineArgs(argc, argv));
}


TEST(CmdLineParserTests, BadNoArgOption) {
    // ARRANGE
    char* argv[] = { "csviewer.exe", "-E", "test.csv" };
    int argc = 3;

    EXPECT_ANY_THROW(ParseCmdLineArgs(argc, argv));
}