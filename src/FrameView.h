#pragma once
#include "CSV.h"
#include <functional>


struct ColumnsLayoutDescription
{
	size_t n_first;  // ������� ������ ������� �� ������� ��������
	bool need_last;  // ���� �� �������� ��������� �������
};

// �������� ������ ������� ��� ������
// �� �����:
//	- �����
//  - ���������� ������ ������ �������
// �� ������:  ����� �� ������, ����� ������� ������ ����� ��������.
typedef std::function<ColumnsLayoutDescription(const std::vector<size_t>&)> ColumnsLayoutPolicyFunc;


// ��������� ������ ������
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

// ����, �������� ������������ ��� ������� �������� ������ ������� ColumnsLayoutPolicyFunc ��� ��������� �������
enum class ConsoleColumnsLayout
{
	FIRST_AND_LAST = 0,              //  col1        ...  colN
	FROM_FIRST_UNTIL_FITS,           //  col1  col2  col3  ...
	FROM_FIRST_UNTIL_FITS_AND_LAST   //  col1  col2  ...  colN
};
// ��������� �������, ��������� ������ ������-������� �� �����
ColumnsLayoutPolicyFunc MakeConsoleColumnsLayoutPolicy(ConsoleColumnsLayout layout);


struct ConsoleFrameViewOptions
{
	ConsoleFrameViewOptions(
		size_t max_col_width = 20,
		size_t max_row_height = 5,
		size_t head_size = 10,
		size_t tail_size = 5,
		bool is_wrap_mode = false,
		std::string col_sep = "   ",
		ConsoleCellTextAlignment align = ConsoleCellTextAlignment::LEFT,
		ConsoleColumnsLayout col_layout = ConsoleColumnsLayout::FIRST_AND_LAST
	) :
		max_col_width(max_col_width),
		max_row_height(max_row_height),
		head_size(head_size),
		tail_size(tail_size),
		is_wrap_mode(is_wrap_mode),
		col_sep(col_sep),
		align(align),
		col_layout(col_layout)
	{}

	size_t max_col_width;  // measured in console text lines
	size_t max_row_height;  // measured in console text lines
	size_t head_size; // measured in console text lines
	size_t tail_size; // measured in console text lines
	bool is_wrap_mode;
	std::string col_sep;
	ConsoleCellTextAlignment align;
	ConsoleColumnsLayout col_layout;
};


class ConsoleFrameView : public IFrameView
{
public:
	ConsoleFrameView(CSVContainer::Frame& frame, const ConsoleFrameViewOptions& options)
		: IFrameView(frame)
		, m_options(options)
		, m_colLayoutPolicyFunc(MakeConsoleColumnsLayoutPolicy(options.col_layout))
	{
		m_columnsMaxTextLength.resize(frame.getNumCols());
		calcColumnsMaxTextLength();
	}
	void renderFrame() override;
	void renderShape() override;
	void renderColumnNames() override;
	void setColumnsLayoutPolicy(ConsoleColumnsLayout layout);

private:
	void calcColumnsMaxTextLength();  // for every column, get maximum length of text (number of Utf-8 symbols) in cells in this column
	size_t renderRow(CSVContainer::RowView row, const std::vector<size_t>& actual_col_widths, const ColumnsLayoutDescription& layout_descr, 
		std::ostringstream& oss);
	size_t renderCell(const Cell& cell, size_t actual_cell_width, std::ostringstream& oss);
	size_t renderGap(std::ostringstream& oss);
	size_t renderColumnSeparator(std::ostringstream& oss);

private:
	ConsoleFrameViewOptions m_options;
	std::vector<size_t> m_columnsMaxTextLength;
	ColumnsLayoutPolicyFunc m_colLayoutPolicyFunc;
};

