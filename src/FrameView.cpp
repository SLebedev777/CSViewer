#include "FrameView.h"
#include "UTF8Utils.h"
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

size_t ConsoleFrameView::renderCell(const Cell& cell, size_t actual_cell_width, std::ostringstream& oss, size_t start)
{
	static const std::string ellipsis("...");
	static const size_t ellipsis_size = Utf8StrLen(ellipsis);
	const size_t cell_size = Utf8StrLen(cell);
	start = (start > cell_size) ? cell_size : start;
	size_t str_size = cell_size - start;
	if (str_size < actual_cell_width)
	{
		std::string pad(actual_cell_width - str_size, ' ');
		std::string str(Utf8SubStr(cell, start, str_size));
		switch (m_options.align)
		{
		case ConsoleCellTextAlignment::RIGHT:
			oss << pad << str;
			break;
		case ConsoleCellTextAlignment::LEFT:
		default:
			oss << str << pad;
			break;
		}
		return start + actual_cell_width;
	}
	else
	{
		if (m_options.is_wrap_mode)
		{
			size_t end = min(cell_size, start + actual_cell_width);
			oss << Utf8SubStr(cell, start, end - start);
			return end;
		}
		else
		{
			size_t final_width = max(0, actual_cell_width - ellipsis_size);
			oss << Utf8SubStr(cell, 0, final_width) << ellipsis;
			return actual_cell_width;
		}
	}
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
		total_width += renderColumnSeparator(oss);
	}

	total_width += renderGap(oss);
	
	if (layout_descr.need_last && c < m_frame.get().getNumCols())
	{
		// move to last column cell in this row
		while (std::next(cell_it) != row.end())
			++cell_it;
		total_width += renderColumnSeparator(oss);
		total_width += renderCell(*cell_it, actual_col_widths.back(), oss);
	}
	oss << std::endl;
	return total_width;
}

size_t ConsoleFrameView::renderRowWrapMode(CSVContainer::RowView row, const std::vector<size_t>& actual_col_widths,
	const ColumnsLayoutDescription& layout_descr, std::ostringstream& oss)
{
	const size_t n_renderable_cols = layout_descr.n_first + size_t(layout_descr.need_last);

	std::vector<std::pair<size_t, size_t>> cell_sizes_and_positions(n_renderable_cols, { 0, 0 });

	// get full text length of every cell in row
	{
		size_t c = 0;
		auto cell_it = row.begin();
		for (; cell_it != row.end(), c < layout_descr.n_first; ++cell_it, ++c)
		{
			cell_sizes_and_positions[c].first = Utf8StrLen(*cell_it);
		}
		if (layout_descr.need_last && c < m_frame.get().getNumCols())
		{
			// move to last column cell in this row
			while (std::next(cell_it) != row.end())
				++cell_it;
			cell_sizes_and_positions[c].first = Utf8StrLen(*cell_it);
		}
	}
	
	size_t total_width = 0;
	size_t num_lines_rendered = 0;
	// render cells line by line until all text is rendered in all row cells
	while (std::any_of(cell_sizes_and_positions.begin(), cell_sizes_and_positions.end(), [](const auto& item) {
		return item.first >= item.second;
		}) && (num_lines_rendered < m_options.max_row_height))
	{
		size_t c = 0;
		total_width = 0;
		
		auto cell_it = row.begin();
		for (; cell_it != row.end(), c < layout_descr.n_first; ++cell_it, ++c)
		{
			size_t start = cell_sizes_and_positions[c].second;
			size_t end = renderCell(*cell_it, actual_col_widths[c], oss, start);
			cell_sizes_and_positions[c].second = end;
			total_width += end - start;
			total_width += renderColumnSeparator(oss);
		}

		total_width += renderGap(oss);

		if (layout_descr.need_last && c < m_frame.get().getNumCols())
		{
			// move to last column cell in this row
			while (std::next(cell_it) != row.end())
				++cell_it;
			total_width += renderColumnSeparator(oss);
			size_t start = cell_sizes_and_positions.back().second;
			size_t end = renderCell(*cell_it, actual_col_widths.back(), oss, start);
			cell_sizes_and_positions.back().second = end;
			total_width += end - start;
		}
		oss << std::endl;
		++num_lines_rendered;
	}
	return total_width;
}

void ConsoleFrameView::renderFrame()
{
	std::ostringstream oss;
	oss << std::endl;
	
	std::vector<size_t> actual_col_widths;
	std::transform(m_columnsMaxTextLength.begin(), m_columnsMaxTextLength.end(), std::back_inserter(actual_col_widths), 
		[this](size_t w) {
			return min(m_options.max_col_width, w);
		});

	auto layout_descr = m_colLayoutPolicyFunc(actual_col_widths, m_options.col_sep.size());

	// pointer to method to render row depending on whether wrap mode is on or off
	size_t (ConsoleFrameView::*pRenderRowFunc)(CSVContainer::RowView, const std::vector<size_t>&, const ColumnsLayoutDescription&,
		std::ostringstream&) = m_options.is_wrap_mode ? &ConsoleFrameView::renderRowWrapMode : &ConsoleFrameView::renderRow;

	// render header according to layout and wrap mode
	size_t total_width = (*this.*pRenderRowFunc)(m_frame.get().getColumnNames(), actual_col_widths, layout_descr, oss);
	std::string header_underline(total_width, '-');
	oss << header_underline << std::endl;

	// render every row according to layout and wrap mode
	for (auto& row : m_frame.get())
	{
		(*this.*pRenderRowFunc)(row, actual_col_widths, layout_descr, oss);
		if (m_options.is_wrap_mode)
			oss << header_underline << std::endl;
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
		[](const std::vector<size_t>& actual_col_widths, size_t)
		{
			ColumnsLayoutDescription result{ 1, true };
			return result;
		}
	);

	auto ConsoleColumnsLayoutPolicy_FromFirstUntilFits(
		[](const std::vector<size_t>& actual_col_widths, size_t col_sep_width)
		{
			auto [console_width, console_height] = GetConsoleSize();
			size_t n_first = 0;
			size_t sum_width = 0;
			while ((n_first < actual_col_widths.size()) && (sum_width < console_width - col_sep_width))
			{
				sum_width += actual_col_widths[n_first] + col_sep_width;
				++n_first;
			}
			ColumnsLayoutDescription result{ n_first - 1, false };
			return result;
		}
	);

	auto ConsoleColumnsLayoutPolicy_FromFirstUntilFitsAndLast(
		[](const std::vector<size_t>& actual_col_widths, size_t col_sep_width)
		{
			auto [console_width, console_height] = GetConsoleSize();
			size_t n_first = 0;
			size_t sum_width = 0;
			while ((n_first < actual_col_widths.size()) && (sum_width < console_width - actual_col_widths.back() - 2*col_sep_width))
			{
				sum_width += actual_col_widths[n_first] + col_sep_width;
				++n_first;
			}
			ColumnsLayoutDescription result{ n_first - 1, true };
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
	case ConsoleColumnsLayout::FROM_FIRST_UNTIL_FITS:
		return ConsoleColumnsLayoutPolicy_FromFirstUntilFits;
	case ConsoleColumnsLayout::FROM_FIRST_UNTIL_FITS_AND_LAST:
		return ConsoleColumnsLayoutPolicy_FromFirstUntilFitsAndLast;
	default:
		return ConsoleColumnsLayoutPolicy_FirstAndLast;
	}
}


void ConsoleFrameView::setColumnsLayoutPolicy(ConsoleColumnsLayout layout)
{
	m_colLayoutPolicyFunc = MakeConsoleColumnsLayoutPolicy(layout);
}