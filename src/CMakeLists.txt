set(BINARY ${CMAKE_PROJECT_NAME})

set(SOURCES)

list(APPEND SOURCES
	"CSViewer.h"
	"CSViewer.cpp"
	"CmdLineParser.h"
	"CmdLineParser.cpp"
	"Settings.h"
)

# Add source to this project's executable.
add_executable (${BINARY} ${SOURCES})

# TODO: Add tests and install targets if needed.
