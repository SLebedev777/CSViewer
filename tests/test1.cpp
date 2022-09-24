#include "gtest/gtest.h"
#include "CmdLineParser.h"


TEST(CSViewerTests, test1) {
    char* argv[] = {"csviewer.exe", "test.csv"};
    int argc = 2;
    auto result = ParseCmdLineArgs(argc, argv);
    CmdLineArgsParseResult expected;
    expected.input_filename = "test.csv";
    EXPECT_EQ(expected.input_filename, result.input_filename);
}