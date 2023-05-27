#include "Command.h"
#include "FrameView.h"
#if (WIN32)
#include <conio.h>
#endif
#include <algorithm>


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
#if (WIN32)
		char c = getch();
#else
		char c;
		std::cin >> c;
#endif
		if (c == 'y' || c == 'Y')
			throw std::runtime_error("Quitting application");
		else if (c == 'n' || c == 'N')
			return;
	}
}

void HelpCommand::Execute()
{
	std::cout << "Available commands:" << std::endl;
	auto sorted_commands = g_ValidPromptCommands;
	std::sort(sorted_commands.begin(), sorted_commands.end(), [](const auto& cmd_a, const auto& cmd_b) {
		return cmd_a.command_full < cmd_b.command_full;
		});
	for (const auto& cmd_descr : sorted_commands)
	{
		std::cout << cmd_descr.command_full << " (short name: " << cmd_descr.command_short << ") - " << cmd_descr.help_string << std::endl;
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
			catch (std::exception& ex)
			{
				throw CommandException("failed to set column name <" + name + "> for index " + std::to_string(index) + ", reason: " + std::string(ex.what()));
			}
		}
	}

	CSVContainer::Frame frame(m_csv);
	for (const auto& name : frame.getColumnNames())
		std::cout << name << ", ";
}


void PrintCommand::Execute()
{
	CSVContainer::Frame frame(m_csv, m_rowRanges, m_colRanges);
	IFrameViewPtr view = std::make_unique<ConsoleFrameView>(frame, m_viewOptions);
	view->renderFrame();
}


ConsoleFrameViewOptionsReflection::ConsoleFrameViewOptionsReflection(ConsoleFrameViewOptions& obj)
	: ClassReflection<ConsoleFrameViewOptions>::ClassReflection(obj)
{
	m_reflection["wrap"] = 
		{
			[this]() { std::cout << "wrap: " << m_obj.is_wrap_mode << std::endl; },
			[this](const std::string& value) { m_obj.is_wrap_mode = static_cast<bool>(std::stoi(value)); }
		};
	m_reflection["index"] =
	{
		[this]() { std::cout << "index: " << m_obj.is_print_row_index << std::endl; },
		[this](const std::string& value) { m_obj.is_print_row_index = static_cast<bool>(std::stoi(value)); }
	};
	m_reflection["chunk_size"] =
	{
		[this]() { std::cout << "chunk_size: " << m_obj.chunk_size << std::endl; },
		[this](const std::string& value) { m_obj.chunk_size = static_cast<size_t>(std::stoi(value)); }
	};

}


void ViewCommand::Execute()
{
	static ConsoleFrameViewOptionsReflection options_reflection(m_viewOptions);

	if (m_cpr.keywords_and_args.empty())
	{
		std::cout << "Current view options: " << std::endl;
		options_reflection.get_all();
		return;
	}

	ConsoleFrameViewOptions new_options(m_viewOptions);
	ConsoleFrameViewOptionsReflection new_options_reflection(new_options);

	for (const auto& arg : m_cpr.keywords_and_args.front().args)
	{
		if (const auto key_value_pair(std::get_if<CommandArgKeyValuePair>(&arg)); key_value_pair)
		{
			const auto param(std::get_if<CommandArgString>(&key_value_pair->first));
			if (!param)
				throw CommandException("MakeViewCommand: wrong key type in key-value pair, should be string");

			const auto value_str(std::get_if<CommandArgString>(&key_value_pair->second));
			const auto value_int(std::get_if<CommandArgNumber>(&key_value_pair->second));

			try
			{
				new_options_reflection.set(*param, std::to_string(*value_int));
			}
			catch (std::runtime_error& ex)
			{
				throw CommandException(ex.what());
			}

		}
		else
			throw CommandException("MakeViewCommand: wrong arg type");
	}

	m_viewOptions = new_options;
}


ICommandPtr MakeCommand(const CommandParseResult& cpr, CSVContainer* csv, ConsoleFrameViewOptions& view_options)
{
	if (cpr.command == "shape")
	{
		return std::make_unique<ShapeCommand>(csv);
	}
	else if (cpr.command == "quit")
	{
		return std::make_unique<QuitCommand>();
	}
	else if (cpr.command == "help")
	{
		return std::make_unique<HelpCommand>();
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
	else if (cpr.command == "view")
	{
		return std::make_unique<ViewCommand>(cpr, view_options);
	}
	else
		throw CommandException("Command not implemented");
}


ICommandPtr MakePrintCommand(const CommandParseResult& cpr, const CSVContainer* csv, const ConsoleFrameViewOptions& view_options)
{
	RangeCollection row_ranges, col_ranges;
	
	for (const auto& kw_args : cpr.keywords_and_args)
	{
		// row
		if (kw_args.kw.empty() || kw_args.kw == "row")
		{
			for (const auto& arg : kw_args.args)
			{
				if (const auto row_range(std::get_if<CommandArgNumberRange>(&arg)); row_range)
				{
					row_ranges.insert(Range(row_range->first, row_range->second + 1));
				}
				else if (const auto row_number(std::get_if<CommandArgNumber>(&arg)); row_number)
				{
					auto row = static_cast<size_t>(*row_number);
					row_ranges.insert(Range(row, row + 1));
				}
			}
		}
		// col
		else if (kw_args.kw == "col")
		{
			for (const auto& arg : kw_args.args)
			{
				if (const auto col_range(std::get_if<CommandArgNumberRange>(&arg)); col_range)
				{
					col_ranges.insert(Range(col_range->first, col_range->second + 1));
				}
				else if (const auto col_number(std::get_if<CommandArgNumber>(&arg)); col_number)
				{
					auto col = static_cast<size_t>(*col_number);
					col_ranges.insert(Range(col, col + 1));
				}
				else if (const auto col_name(std::get_if<CommandArgString>(&arg)); col_name)
				{
					size_t index = csv->findColumnIndex(*col_name);
					if (index == CSVContainer::END)
						throw CommandException("MakePrintCommand: wrong column name: " + *col_name);

					col_ranges.insert(Range(index, index + 1));
				}
				else if (const auto col_str_range(std::get_if<CommandArgStringRange>(&arg)); col_str_range)
				{
					size_t index_from = csv->findColumnIndex(col_str_range->first);
					if (index_from == CSVContainer::END)
						throw CommandException("MakePrintCommand: wrong column name: " + col_str_range->first);

					size_t index_to = csv->findColumnIndex(col_str_range->second);
					if (index_to == CSVContainer::END)
						throw CommandException("MakePrintCommand: wrong column name: " + col_str_range->second);

					col_ranges.insert(Range(index_from, index_to + 1));
				}

			}
		}
		else
			throw CommandException("MakePrintCommand: unknown keyword: " + kw_args.kw);
	}

	CSVContainer::Frame frame(csv);
	row_ranges = row_ranges.empty() ? frame.getRowRanges() : row_ranges;
	col_ranges = col_ranges.empty() ? frame.getColRanges() : col_ranges;

	return std::make_unique<PrintCommand>(csv, view_options, row_ranges, col_ranges);

}


ICommandPtr MakeColsCommand(const CommandParseResult& cpr, CSVContainer* csv)
{
	std::vector<std::pair<size_t, std::string>> index_names;
	if (cpr.keywords_and_args.empty())
		return std::make_unique<ColsCommand>(csv, index_names);

	if (cpr.keywords_and_args.front().kw == "header")
	{
		CSVContainer::Frame header(csv, 0, 2);
		for (auto& row : header)
		{
			size_t index = 0;
			for (auto& cell : row)
			{
				index_names.push_back({index, cell});
				index++;
			}
			break;
		}
		return std::make_unique<ColsCommand>(csv, index_names);
	}

	size_t arg_index = 0;
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
		else if (const auto name(std::get_if<CommandArgString>(&arg)); name)
		{
			index_names.emplace_back(arg_index, *name);
		}
		else
			throw CommandException("MakeColsCommand: wrong arg type");
		
		arg_index++;
	}
	return std::make_unique<ColsCommand>(csv, index_names);
}
