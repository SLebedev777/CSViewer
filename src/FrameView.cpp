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
#include <algorithm>
#include <sstream>
#include <numeric>


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


size_t ConsoleFrameView::renderColumnSeparator(std::ostringstream& oss)
{
	oss << m_options.col_sep;
	return m_options.col_sep.size();
}

size_t ConsoleFrameView::renderGap(std::ostringstream& oss)
{
	static const std::string gap("    ...    ");
	oss << gap;
	return gap.size();
}

size_t ConsoleFrameView::renderCell(const Cell& cell, size_t actual_cell_width, std::ostringstream& oss)
{
	static const std::string ellipsis("...");
	if (cell.size() < actual_cell_width)
	{
		std::string pad(actual_cell_width - cell.size(), ' ');
		oss << cell << pad;
	}
	else
	{
		size_t final_width = max(0, actual_cell_width - ellipsis.size());
		oss << cell.substr(0, final_width) << ellipsis;
	}
	return actual_cell_width;
}

size_t ConsoleFrameView::renderRow(CSVContainer::RowView row, const std::vector<size_t>& actual_col_widths, 
	const ColumnsLayoutDescription& layout_descr, std::ostringstream& oss)
{
	size_t c = 0;
	size_t total_width = 0;
	auto cell_it = row.begin();
	for (; cell_it != row.end(), c < layout_descr.n_first; ++cell_it, ++c)
	{
		total_width += renderCell(*cell_it, actual_col_widths[c], oss);
	}

	total_width += renderGap(oss);
	
	if (layout_descr.need_last && c < m_frame.get().getNumCols())
	{
		// move to last column cell in this row
		while (std::next(cell_it) != row.end())
			++cell_it;
		total_width += renderCell(*cell_it, actual_col_widths.back(), oss);
	}
	oss << std::endl;
	return total_width;
}

void ConsoleFrameView::renderFrame()
{
	std::ostringstream oss;
	
	std::vector<size_t> actual_col_widths;
	std::transform(m_columnsMaxTextLength.begin(), m_columnsMaxTextLength.end(), std::back_inserter(actual_col_widths), 
		[this](size_t w) {
			return min(m_options.max_col_width, w);
		});

	auto layout_descr = m_colLayoutPolicyFunc(actual_col_widths);

	// render header according to layout
	size_t total_width = renderRow(m_frame.get().getColumnNames(), actual_col_widths, layout_descr, oss);
	std::string header_underline(total_width, '-');
	oss << header_underline << std::endl;

	// render every row according to layout
	for (auto& row : m_frame.get())
	{
		renderRow(row, actual_col_widths, layout_descr, oss);
	}

	std::cout << oss.str() << std::endl;
	
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

void ConsoleFrameView::calcColumnsMaxTextLength()
{
	const auto num_cols = m_frame.get().getNumCols();
	std::fill(m_columnsMaxTextLength.begin(), m_columnsMaxTextLength.end(), 0);
	std::vector<size_t> row_widths(num_cols, 0);
	for (const auto& row : m_frame.get())
	{
		std::transform(row.cbegin(), row.cend(), row_widths.begin(), Utf8StrLen);
		for (size_t i = 0; i < num_cols; ++i)
		{
			if (row_widths[i] > m_columnsMaxTextLength[i])
				m_columnsMaxTextLength[i] = row_widths[i];
		}
	}
}

namespace
{
	auto ConsoleColumnsLayoutPolicy_FirstAndLast(
		[](const std::vector<size_t>& actual_col_widths)
		{
			ColumnsLayoutDescription result{ 1, true };
			return result;
		}
	);
}

ColumnsLayoutPolicyFunc MakeConsoleColumnsLayoutPolicy(ConsoleColumnsLayout layout)
{
	switch (layout)
	{
	case ConsoleColumnsLayout::FIRST_AND_LAST:
		return ConsoleColumnsLayoutPolicy_FirstAndLast;
	default:
		return ConsoleColumnsLayoutPolicy_FirstAndLast;
	}
}


void ConsoleFrameView::setColumnsLayoutPolicy(ConsoleColumnsLayout layout)
{
	m_colLayoutPolicyFunc = MakeConsoleColumnsLayoutPolicy(layout);
}