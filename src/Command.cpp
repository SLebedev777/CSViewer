#include "Command.h"
#include "FrameView.h"


void ShapeCommand::Execute()
{
	ConsoleFrameViewOptions view_options;
	IFrameViewPtr view = std::make_unique<ConsoleFrameView>(m_frame, view_options);
	view->renderShape();
}
