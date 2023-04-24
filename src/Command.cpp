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


void ColsCommand::Execute()
{
	if (!m_indexNames.empty())
	{
		for (const auto& [index, name] : m_indexNames)
		{
			try
			{
				m_csv->setColumnName(index, name);
			}
			catch (std::out_of_range& ex)
			{
				throw CommandException("failed to set column name <" + name + "> for index " + std::to_string(index));
			}
		}
	}

	CSVContainer::Frame frame(m_csv);
	for (const auto& name : frame.getColumnNames())
		std::cout << name << ", ";
}


void PrintCommand::Execute()
{
	throw CommandException("PrintCommand not implemented");
}


ICommandPtr MakeCommand(const CommandParseResult& cpr, CSVContainer* csv, const ConsoleFrameViewOptions& view_options)
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
	else if (cpr.command == "cols")
	{
		return MakeColsCommand(cpr, csv);
	}
	else if (cpr.command == "print")
	{
		return MakePrintCommand(cpr, csv, view_options);
	}
	else
		throw CommandException("Command not implemented");
}


ICommandPtr MakePrintCommand(const CommandParseResult& cpr, const CSVContainer* csv, const ConsoleFrameViewOptions& view_options)
{
	RangeCollection row_ranges, col_ranges;
	return std::make_unique<PrintCommand>(csv, view_options, row_ranges, col_ranges);

}


ICommandPtr MakeColsCommand(const CommandParseResult& cpr, CSVContainer* csv)
{
	std::vector<std::pair<size_t, std::string>> index_names;
	if (cpr.keywords_and_args.empty())
		return std::make_unique<ColsCommand>(csv, index_names);

	for (const auto& arg : cpr.keywords_and_args.front().args)
	{
		if (const auto key_value_pair(std::get_if<CommandArgKeyValuePair>(&arg)); key_value_pair)
		{
			const auto new_name(std::get_if<CommandArgString>(&key_value_pair->second));
			if (!new_name)
				throw CommandException("MakeColsCommand: wrong value type in key-value pair");
			
			if (const auto index(std::get_if<CommandArgNumber>(&key_value_pair->first)); index)
			{
				index_names.emplace_back(*index, *new_name);
			}
			else if (const auto old_name(std::get_if<CommandArgString>(&key_value_pair->first)); old_name)
			{
				size_t index = csv->findColumnIndex(*old_name);
				if (index == CSVContainer::END)
					throw CommandException("MakeColsCommand: wrong old column name in key-value pair");
				
				index_names.emplace_back(index, *new_name);
			}
			else
				throw CommandException("MakeColsCommand: wrong key type in key-value pair");
		}
	}
	return std::make_unique<ColsCommand>(csv, index_names);
}