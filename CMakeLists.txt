# CMakeList.txt : CMake project for CSViewer, include source and define
# project specific logic here.
#
cmake_minimum_required (VERSION 3.8)

set(CMAKE_CXX_STANDARD 17)

project ("CSViewer")

# Add source to this project's executable.
add_executable (CSViewer "CSViewer.cpp" "CSViewer.h" "CmdLineParser.h" "CmdLineParser.cpp" "Settings.h")

# TODO: Add tests and install targets if needed.
