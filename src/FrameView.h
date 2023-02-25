#pragma once
#include "CSV.h"


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

enum class ConsoleColumnsDisplayPolicy
{
	FIRST_AND_LAST = 0,              //  col1        ...  colN
	FROM_FIRST_UNTIL_FITS,           //  col1  col2  col3  ...
	FROM_FIRST_UNTIL_FITS_AND_LAST   //  col1  col2  ...  colN
};

struct ConsoleFrameViewOptions
{
	ConsoleFrameViewOptions(
		size_t max_col_width = 20,
		size_t max_row_height = 5,
		size_t max_rows = 50,
		bool is_wrap_mode = false,
		std::string col_sep = "   ",
		ConsoleCellTextAlignment align = ConsoleCellTextAlignment::LEFT,
		ConsoleColumnsDisplayPolicy col_policy = ConsoleColumnsDisplayPolicy::FROM_FIRST_UNTIL_FITS
	) :
		max_col_width(max_col_width),
		max_row_height(max_row_height),
		max_rows(max_rows),
		is_wrap_mode(is_wrap_mode),
		col_sep(col_sep),
		align(align),
		col_policy(col_policy)
	{}

	size_t max_col_width;  // measured in console text lines
	size_t max_row_height;  // measured in console text lines
	size_t max_rows; // measured in console text lines
	bool is_wrap_mode;
	std::string col_sep;
	ConsoleCellTextAlignment align;
	ConsoleColumnsDisplayPolicy col_policy;
};


class ConsoleFrameView : public IFrameView
{
public:
	ConsoleFrameView(CSVContainer::Frame& frame, const ConsoleFrameViewOptions& options)
		: IFrameView(frame)
		, m_options(options)
	{}
	void renderFrame() override;
	void renderShape() override;
	void renderColumnNames() override;

private:
	ConsoleFrameViewOptions m_options;
};

