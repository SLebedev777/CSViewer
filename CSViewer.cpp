// CSViewer.cpp : Defines the entry point for the application.
//

#include "CmdLineParser.h"
#include "CSViewer.h"
#include <sstream>

using namespace std;

int main(int argc, char** argv)
{	
	try
	{
		auto args_parse_result = ParseCmdLineArgs(argc, argv);

		std::cout << "Input filename: " << args_parse_result.input_filename << std::endl;

		for (const auto& option : args_parse_result.options)
			std::visit(CmdLineOptionParseResultVariantPrinter, option);
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
