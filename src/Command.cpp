#include "Command.h"
#include "FrameView.h"
#include <conio.h>


void ShapeCommand::Execute()
{
	ConsoleFrameViewOptions view_options;
	CSVContainer::Frame frame(m_csv);
	IFrameViewPtr view = std::make_unique<ConsoleFrameView>(frame, view_options);
	view->renderShape();
}

void QuitCommand::Execute()
{
	std::cout << "Are you sure you want to quit? (Y/N)";
	while (true)
	{
		char c = getch();
		if (c == 'y' || c == 'Y')
			throw std::runtime_error("Quitting application");
		else if (c == 'n' || c == 'N')
			return;
	}
}

void HeadCommand::Execute()
{
	CSVContainer::Frame head(m_csv, 0, m_nRows);
	IFrameViewPtr view_head = std::make_unique<ConsoleFrameView>(head, m_viewOptions);
	view_head->renderFrame();
}


void TailCommand::Execute()
{
	CSVContainer::Frame tail(m_csv, CSVContainer::Frame(m_csv).getNumRows() - m_nRows);
	IFrameViewPtr view_tail = std::make_unique<ConsoleFrameView>(tail, m_viewOptions);
	view_tail->renderFrame();
}


ICommandPtr MakeCommand(const CommandParseResult& cpr, const CSVContainer* csv, const ConsoleFrameViewOptions& view_options)
{
	if (cpr.command == "shape")
	{
		return std::make_unique<ShapeCommand>(csv);
	}
	else if (cpr.command == "quit")
	{
		return std::make_unique<QuitCommand>();
	}
	else if (cpr.command == "head")
	{
		size_t n_rows = 10;
		if (!cpr.keywords_and_args.empty())
		{
			if (!cpr.keywords_and_args.front().args.empty())
				n_rows = std::get<CommandArgNumber>(cpr.keywords_and_args.front().args.front());
		}
		return std::make_unique<HeadCommand>(csv, view_options, n_rows);
	}
	else if (cpr.command == "tail")
	{
		size_t n_rows = 10;
		if (!cpr.keywords_and_args.empty())
		{
			if (!cpr.keywords_and_args.front().args.empty())
				n_rows = std::get<CommandArgNumber>(cpr.keywords_and_args.front().args.front());
		}
		return std::make_unique<TailCommand>(csv, view_options, n_rows);
	}
	else
		throw std::runtime_error("Command not implemented");
}