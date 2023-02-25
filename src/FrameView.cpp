#include "FrameView.h"
#include <iostream>
#include <utility>
#ifdef WIN32
#define WIN32_LEAN_AND_MEAN
#define VC_EXTRALEAN
#include <Windows.h>
#elif defined __linux__
#include <sys/ioctl.h>
#endif // Windows/Linux

namespace
{
	std::pair<int, int> GetConsoleSize()
	{
		int width = 0, height = 0;
#ifdef WIN32
		CONSOLE_SCREEN_BUFFER_INFO csbi;
		GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi);
		width = csbi.srWindow.Right - csbi.srWindow.Left + 1;
		height = csbi.srWindow.Bottom - csbi.srWindow.Top + 1;
#elif defined __linux__
		struct winsize w;
		ioctl(fileno(stdout), TIOCGWINSZ, &w);
		width = (int)(w.ws_col);
		height = (int)(w.ws_row);
#endif
		return { width, height };
	}

	size_t Utf8StrLen(const std::string& utf8_str)
	{
		size_t len = 0;
		const char* p = utf8_str.c_str();
		while (*p)
		{
			if ((*p++ & 0xc0) != 0x80)  // count every first octet (beginning of the block) - it never starts with bits 10
				++len;
		}
		return len;
	}
}


void ConsoleFrameView::renderFrame()
{
	std::cout << std::endl;
	renderColumnNames();
	std::cout << "--------------------" << std::endl;
	for (const auto& row : m_frame.get())
	{
		std::cout << row << std::endl;
	}
	renderShape();
}

void ConsoleFrameView::renderShape()
{
	auto frame = m_frame.get();
	std::cout << std::endl;
	std::cout << "Shape: " << frame.getRowRanges().totalElements() << " rows x " <<
		frame.getColRanges().totalElements() << " cols" << std::endl;
}

void ConsoleFrameView::renderColumnNames()
{
	std::cout << m_frame.get().getColumnNames() << std::endl;
}