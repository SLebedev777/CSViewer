﻿set(BINARY ${CMAKE_PROJECT_NAME}_tests)

set(SOURCES)

list(APPEND SOURCES
	"main.cpp"
	"CmdLineParserTests.cpp"
	"RangeTests.cpp"
	"CSVContainerTests.cpp"
	"LibiconvTests.cpp"
	"UTF8UtilsTests.cpp"
	"PromptParserTests.cpp"
)

# VERY IMPORTANT!!! THIS IS NEEDED THAT GTEST WILL NOT OVERRIDE MAIN PROJECT'S COMPILING&LINKING SETTINGS!
if (MSVC)
    set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
endif()

# Add source to this project's executable.
add_executable (${BINARY} ${SOURCES})

add_test(NAME ${BINARY} COMMAND ${BINARY})

include_directories(../externals)

target_link_libraries(${BINARY} PUBLIC ${CMAKE_PROJECT_NAME}_lib gtest_main)

# copy test csv files from \data subdirectory to target dir
add_custom_command(
  TARGET ${BINARY} POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy_directory 
    ${CMAKE_CURRENT_SOURCE_DIR}/data 
    $<TARGET_FILE_DIR:${BINARY}>/data)