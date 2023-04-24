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


// factory
ICommandPtr MakeCommand(const CommandParseResult& cpr, const CSVContainer* csv, const ConsoleFrameViewOptions& view_options);
