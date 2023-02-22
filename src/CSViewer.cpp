// CSViewer.cpp : Defines the entry point for the application.
//

#include "CmdLineParser.h"
#include "CSViewer.h"
#include "FrameView.h"
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

		CSVLoadingSettings settings = MakeSettingsByCmdLineArgs(args_parse_result);

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

		auto csv = CSVContainer(settings);

		describe_frame(CSVContainer::Frame(&csv));
		CSVContainer::Frame head(&csv, 0, 10);
		std::cout << "Head preview: " << std::endl;
		print_frame(head);

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
