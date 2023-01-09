// CSViewer.cpp : Defines the entry point for the application.
//

#include "CmdLineParser.h"
#include "CSViewer.h"
#include <sstream>
#ifdef WIN32
#include <windows.h>
#endif

using namespace std;

int main(int argc, char** argv)
{
#ifdef WIN32
	SetConsoleCP(CP_UTF8);
	SetConsoleOutputCP(CP_UTF8);
#endif

	try
	{
		auto args_parse_result = ParseCmdLineArgs(argc, argv);

		std::cout << "Input filename: " << args_parse_result.input_filename << std::endl;

		for (const auto& option : args_parse_result.options)
			std::visit(CmdLineOptionParseResultVariantPrinter, option);

		// model = LoadCSV(args_parse_result);
		// while(true)
		// {
		// prompt();
		// command = ParseCommand(cin);
		// if (command == QUIT)  break;
		// result = model->execute(command);
		// view = View(result);
		// view->print();
		// }
	}
	catch (std::exception& ex)
	{
		std::cout << ex.what() << std::endl;
		return 1;
	}
	catch (...)
	{
		return 1;
	}

	return 0;
}
