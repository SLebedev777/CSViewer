#pragma once
#include "CSV.h"
#include <functional>
#include <memory>


struct ColumnsLayoutDescription
{
	size_t n_first;  // сколько первых колонок по порядку рисовать
	bool need_last;  // надо ли рисовать последнюю колонку
};

// политика выбора колонок для вьюера
// на входе:
//	- фрейм
//  - актуальная ширина каждой колонки
//  - ширина разделителя колонок
// на выходе:  ответ на вопрос, какие колонки фрейма будем рисовать.
typedef std::function<ColumnsLayoutDescription(const std::vector<size_t>&, size_t)> ColumnsLayoutPolicyFunc;


// интерфейс вьюера фрейма
class IFrameView
{
public:
	IFrameView(CSVContainer::Frame& frame)
		: m_frame(frame)
	{}
	virtual ~IFrameView() {};
	virtual void renderFrame() = 0;
	virtual void renderShape() = 0;
	virtual void renderColumnNames() = 0;

protected:
	std::reference_wrapper<CSVContainer::Frame> m_frame;
};

typedef std::unique_ptr<IFrameView> IFrameViewPtr;


// Frame view implementation for OS console

enum class ConsoleCellTextAlignment
{
	LEFT = 0,
	RIGHT
};

// энум, задающий соответствие для функции политики выбора колонок ColumnsLayoutPolicyFunc для системной консоли
enum class ConsoleColumnsLayout
{
	FIRST_AND_LAST = 0,              //  col1        ...  colN
	FROM_FIRST_UNTIL_FITS,           //  col1  col2  col3  ...
	FROM_FIRST_UNTIL_FITS_AND_LAST   //  col1  col2  ...  colN
};
// фабричная функция, создающая нужную лямбда-функцию по энуму
ColumnsLayoutPolicyFunc MakeConsoleColumnsLayoutPolicy(ConsoleColumnsLayout layout);


struct ConsoleFrameViewOptions
{
	ConsoleFrameViewOptions(
		size_t max_col_width = 20,
		size_t max_row_height = std::numeric_limits<size_t>::max(),
		size_t chunk_size = 50,
		bool is_wrap_mode = true,
		bool is_print_row_index = true,
		std::string col_sep = "  |  ",
		ConsoleCellTextAlignment align = ConsoleCellTextAlignment::LEFT,
		ConsoleColumnsLayout col_layout = ConsoleColumnsLayout::FIRST_AND_LAST
	) :
		max_col_width(max_col_width),
		max_row_height(max_row_height),
		chunk_size(chunk_size),
		is_wrap_mode(is_wrap_mode),
		is_print_row_index(is_print_row_index),
		col_sep(col_sep),
		align(align),
		col_layout(col_layout)
	{}

	size_t max_col_width;  // measured in console characters
	size_t max_row_height;  // measured in console text lines
	size_t chunk_size; // measured in number of rows rendered (in wrap mode, a row can occupy many lines)
	bool is_wrap_mode;
	bool is_print_row_index;
	std::string col_sep;
	ConsoleCellTextAlignment align;
	ConsoleColumnsLayout col_layout;
};


class ConsoleFrameView : public IFrameView
{
public:
	ConsoleFrameView(CSVContainer::Frame& frame, const ConsoleFrameViewOptions& options);
	void renderFrame() override;
	void renderShape() override;
	void renderColumnNames() override;
	void setColumnsLayoutPolicy(ConsoleColumnsLayout layout);

private:
	void calcColumnsMaxTextLength();  // for every column, get maximum length of text (number of Utf-8 symbols) in cells in this column
	size_t renderRow(CSVContainer::RowView row, const std::vector<size_t>& actual_col_widths, const ColumnsLayoutDescription& layout_descr, 
		std::ostringstream& oss, bool is_print_row_index = false);
	size_t renderRowWrapMode(CSVContainer::RowView row, const std::vector<size_t>& actual_col_widths, const ColumnsLayoutDescription& layout_descr,
		std::ostringstream& oss, bool is_print_row_index = false);
	// In wrap_mode = false, returns number of symbols rendered. In wrap_mode = true, returns current end position of rendered piece of cell text.
	size_t renderCell(const Cell& cell, size_t actual_cell_width, std::ostringstream& oss, size_t start = 0);
	size_t renderGap(std::ostringstream& oss);
	size_t renderColumnSeparator(std::ostringstream& oss);
	size_t getNumColumns() const;

private:
	ConsoleFrameViewOptions m_options;
	std::vector<size_t> m_columnsMaxTextLength;
	ColumnsLayoutPolicyFunc m_colLayoutPolicyFunc;
};

