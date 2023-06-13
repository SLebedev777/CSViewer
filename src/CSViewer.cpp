﻿// CSViewer.cpp : Defines the entry point for the application.
//

#include "CmdLineParser.h"
#include "CSViewer.h"
#include "FrameView.h"
#include "PromptParser.h"
#include "Command.h"
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

		if (csviewer_internal::IsHelpOptionExist(args_parse_result.options))
			PrintCmdLineHelp();

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

		CSVContainer::Frame frame(&csv);
		ConsoleFrameViewOptions view_options;
		IFrameViewPtr view = std::make_unique<ConsoleFrameView>(frame, view_options);
		view->renderShape();

		CSVContainer::Frame head(&csv, 0, 10);
		view_options.col_layout = ConsoleColumnsLayout::FROM_FIRST_UNTIL_FITS_AND_LAST;
		IFrameViewPtr view_head = std::make_unique<ConsoleFrameView>(head, view_options);
		view_head->renderFrame();

		// main loop
		while (true)
		{
			std::cin.clear();
			std::cout << std::endl << ">>>";
			std::string input;
			std::getline(std::cin, input);
			CommandParseResult command_parse_result;
			try
			{
				command_parse_result = ParsePromptInput(input);
				std::cout << command_parse_result;

				auto command = MakeCommand(command_parse_result, &csv, view_options);
				command->Execute();
			}
			catch (PromptParserException& ex)
			{
				std::cout << ex.what() << std::endl;
			}
			catch (CommandException& ex)
			{
				std::cout << ex.what() << std::endl;
			}
		}
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
