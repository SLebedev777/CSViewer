﻿set(BINARY ${CMAKE_PROJECT_NAME}_tests)

set(SOURCES)

list(APPEND SOURCES
	"main.cpp"
	"test1.cpp"
)

# VERY IMPORTANT!!! THIS IS NEEDED THAT GTEST WILL NOT OVERRIDE MAIN PROJECT'S COMPILING&LINKIG SETTINGS!
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)

# Add source to this project's executable.
add_executable (${BINARY} ${SOURCES})

add_test(NAME ${BINARY} COMMAND ${BINARY})

target_link_libraries(${BINARY} PUBLIC ${CMAKE_PROJECT_NAME}_lib gtest_main)