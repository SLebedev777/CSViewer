# CMakeList.txt : CMake project for CSViewer, include source and define
# project specific logic here.
#
cmake_minimum_required (VERSION 3.10)

set(CMAKE_CXX_STANDARD 17)

project ("CSViewer")

include_directories(src)

# for gtest linking on Windows
if(MSVC)
    add_compile_options(
        $<$<CONFIG:>:/MD>
        $<$<CONFIG:Debug>:/MDd>
        $<$<CONFIG:Release>:/MD>
    )
endif()

add_subdirectory(externals/libiconv)
add_subdirectory(externals/googletest)
add_subdirectory(src)
add_subdirectory(tests)


# TODO: Add tests and install targets if needed.
