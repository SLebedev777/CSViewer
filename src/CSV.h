#ifndef __CSVIEWER_CSV_H__
#define __CSVIEWER_CSV_H__

#include <vector>
#include <string>
#include <stdexcept>
#include <limits>
#include "CmdLineParser.h"
#include "Range.h"


enum class BadLinesPolicy
{
	BL_RAISE,
	BL_WARN,
	BL_SKIP
};


struct CSVLoadingSettings
{
	explicit CSVLoadingSettings(const std::string& filename,
		const std::string& encoding = "utf8",
		char delimiter = ',',
		char quote = '"',
		bool read_header = false,
		BadLinesPolicy bad_lines_policy = BadLinesPolicy::BL_RAISE,
		size_t skip_first_lines = 0
	)
		: filename(filename)
		, encoding(encoding)
		, delimiter(delimiter)
		, quote(quote)
		, read_header(read_header)
		, bad_lines_policy(bad_lines_policy)
		, skip_first_lines(skip_first_lines)
	{}

	std::string filename;
	std::string encoding;
	char delimiter;
	char quote;
	bool read_header;
	BadLinesPolicy bad_lines_policy;
	size_t skip_first_lines;
};


using Cell = std::string;
using Row = std::vector<Cell>;

class CSVContainer
{
public:
	static const size_t END = std::numeric_limits<size_t>::max();

	CSVContainer(const CSVLoadingSettings& settings);

	class RowView;

	class cell_iterator
	{
	public:
		cell_iterator(const RowView* row_view);
		const Cell& operator*();
		cell_iterator& operator++();
	
	private:
		const RowView* row_view;
	};


	class RowView
	{
	public:
		RowView(const Row* row, const RangeCollection* col_ranges)
			: row(row), col_ranges(col_ranges)
		{}

		CSVContainer::cell_iterator begin();
		CSVContainer::cell_iterator end();

	private:
		const Row* row;
		const RangeCollection* col_ranges;
	};


	class row_iterator
	{
	public:
		using difference_type = std::ptrdiff_t;
		using value_type = RowView;
		using pointer = const value_type*;
		using reference = const value_type&;
		using iterator_category = std::forward_iterator_tag;

		row_iterator(const CSVContainer* csv, const RangeCollection* row_ranges, size_t pos);
		reference operator*();
		row_iterator& operator++();

	private:
		pointer row;
		const RangeCollection* row_ranges;
	};

	// proxy class that provides read-only access to subsample of our CSV by rows and columns.
	// it is intermediate class for building a view of csv subsample.
	// NO copy of any piece of data is made.
	class Frame
	{
		friend class CSVContainer;

	public:
		static const bool AXIS_ROWS = false;
		static const bool AXIS_COLS = true;

		Frame(const CSVContainer* csv, size_t row_from = 0, size_t row_to = END, size_t col_from = 0, size_t col_to = END);
		Frame(const CSVContainer* csv, const Range& range, bool axis = AXIS_ROWS);
		Frame(const CSVContainer* csv, const Range& rows, const Range& cols);
		Frame(const CSVContainer* csv, const RangeCollection& ranges, bool axis = AXIS_ROWS);
		Frame(const CSVContainer* csv, RangeCollection&& ranges, bool axis = AXIS_ROWS);
		Frame(const CSVContainer* csv, const RangeCollection& row_ranges, const RangeCollection& col_ranges);
		Frame(const CSVContainer* csv, RangeCollection&& row_ranges, RangeCollection&& col_ranges);

		// for (const auto& row : frame)
		//     for (const auto& cell : row)
		//         ...
		row_iterator begin();
		row_iterator end();

	private:
		const CSVContainer* csv;
		RangeCollection row_ranges;
		RangeCollection col_ranges;
	};

private:
	CsvLoadingSettings m_settings;
	std::vector<Row> m_table;
	std::vector<std::string> m_columnNames;
	size_t m_numRows;
	size_t m_numCols;
};

#endif