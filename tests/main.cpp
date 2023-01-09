#include "gtest/gtest.h"
#ifdef WIN32
#include <windows.h>
#endif


int main(int argc, char** argv) {

#ifdef WIN32
    SetConsoleCP(CP_UTF8);
    SetConsoleOutputCP(CP_UTF8);
#endif
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}