#pragma once
#include "CSV.h"

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
	ShapeCommand(CSVContainer::Frame& frame)
		: m_frame(frame)
	{}
	void Execute() override;

private:
	std::reference_wrapper<CSVContainer::Frame> m_frame;
};