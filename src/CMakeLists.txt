set(BINARY ${CMAKE_PROJECT_NAME})

set(SOURCES)

list(APPEND SOURCES
	"IConvConverter.h"
	"IConvConverter.cpp"
	"StringUtils.h"
	"UTF8Utils.h"
	"UTF8Utils.cpp"
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
	"PromptParser.h"
	"PromptParser.cpp"
	"ClassReflection.h"
	"Command.h"
	"Command.cpp"
)


include_directories(../externals)

message(Binary dir: ${CMAKE_BINARY_DIR})

# Add source to this project's executable.
add_executable (${BINARY} ${SOURCES})
target_link_libraries(${BINARY} libiconv)

add_library(${BINARY}_lib STATIC ${SOURCES})
target_link_libraries(${BINARY}_lib libiconv)

