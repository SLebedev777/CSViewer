#pragma once
#include "CSV.h"
#include "PromptParser.h"
#include "FrameView.h"
#include <memory>

class ICommand
{
public:
	virtual ~ICommand() {};
	virtual void Execute() = 0;
};

typedef std::unique_ptr<ICommand> ICommandPtr;


class ShapeCommand : public ICommand
{
public:
	ShapeCommand(const CSVContainer* csv)
		: m_csv(csv)
	{}
	void Execute() override;

private:
	const CSVContainer* m_csv;
};


class QuitCommand : public ICommand
{
public:
	QuitCommand() = default;
	void Execute() override;
};


class HeadCommand : public ICommand
{
public:
	HeadCommand(const CSVContainer* csv, const ConsoleFrameViewOptions& view_options, size_t n_rows)
		: m_csv(csv)
		, m_viewOptions(view_options)
		, m_nRows(n_rows)
	{}
	void Execute() override;

private:
	const CSVContainer* m_csv;
	ConsoleFrameViewOptions m_viewOptions;
	size_t m_nRows;
};


class TailCommand : public ICommand
{
public:
	TailCommand(const CSVContainer* csv, const ConsoleFrameViewOptions& view_options, size_t n_rows)
		: m_csv(csv)
		, m_viewOptions(view_options)
		, m_nRows(n_rows)
	{}
	void Execute() override;

private:
	const CSVContainer* m_csv;
	ConsoleFrameViewOptions m_viewOptions;
	size_t m_nRows;
};


class ColsCommand : public ICommand
{
public:
	ColsCommand(CSVContainer* csv, const std::vector<std::pair<size_t, std::string>>& index_names)
		: m_csv(csv)
		, m_indexNames(index_names)
	{}
	void Execute() override;

private:
	CSVContainer* m_csv; // non-const
	std::vector<std::pair<size_t, std::string>> m_indexNames;
};


class PrintCommand : public ICommand
{
public:
	PrintCommand(const CSVContainer* csv, const ConsoleFrameViewOptions& view_options,
		const RangeCollection& row_ranges, const RangeCollection& col_ranges)
		: m_csv(csv)
		, m_viewOptions(view_options)
		, m_rowRanges(row_ranges)
		, m_colRanges(col_ranges)
	{}
	void Execute() override;

private:
	const CSVContainer* m_csv;
	ConsoleFrameViewOptions m_viewOptions;
	RangeCollection m_rowRanges;
	RangeCollection m_colRanges;
};


// factory
ICommandPtr MakeCommand(const CommandParseResult& cpr, CSVContainer* csv, const ConsoleFrameViewOptions& view_options);
ICommandPtr MakePrintCommand(const CommandParseResult& cpr, const CSVContainer* csv, const ConsoleFrameViewOptions& view_options);
ICommandPtr MakeColsCommand(const CommandParseResult& cpr, CSVContainer* csv);


class CommandException : public std::runtime_error
{
	using std::runtime_error::runtime_error;
};