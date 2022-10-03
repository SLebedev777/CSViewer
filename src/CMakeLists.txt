set(BINARY ${CMAKE_PROJECT_NAME})

set(SOURCES)

list(APPEND SOURCES
	"CSViewer.h"
	"CSViewer.cpp"
	"CmdLineParser.h"
	"CmdLineParser.cpp"
	"Settings.h"
	"Range.h"
	"Range.cpp"
	"CSV.h"
	"CSV.cpp"
)

# Add source to this project's executable.
add_executable (${BINARY} ${SOURCES})

add_library(${BINARY}_lib STATIC ${SOURCES})
