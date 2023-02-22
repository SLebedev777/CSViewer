set(BINARY ${CMAKE_PROJECT_NAME})

set(SOURCES)

list(APPEND SOURCES
	"IConvConverter.h"
	"IConvConverter.cpp"
	"CSViewer.h"
	"CSViewer.cpp"
	"CmdLineParser.h"
	"CmdLineParser.cpp"
	"Settings.h"
	"Range.h"
	"Range.cpp"
	"CSV.h"
	"CSV.cpp"
	"FrameView.h"
	"FrameView.cpp"
)

include_directories(../externals)

message(Binary dir: ${CMAKE_BINARY_DIR})

find_library(libiconv libiconv ${CMAKE_BINARY_DIR}/externals/libiconv)
find_library(libcharset libcharset ${CMAKE_BINARY_DIR}/externals/libiconv)

# Add source to this project's executable.
add_executable (${BINARY} ${SOURCES})
target_link_libraries(${BINARY} ${libiconv} ${libcharset})

add_library(${BINARY}_lib STATIC ${SOURCES})
target_link_libraries(${BINARY}_lib ${libiconv} ${libcharset})

