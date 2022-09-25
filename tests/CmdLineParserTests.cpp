#include "gtest/gtest.h"
#include "CmdLineParser.h"


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