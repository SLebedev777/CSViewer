#pragma once

#include <vector>
#include <string>
#include <stdexcept>
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

class CSVContainer
{
public:
	static const size_t END = static_cast<size_t>(-1);

	CSVContainer(const CSVLoadingSettings& settings);

	class cell_iterator
	{
	public:
		cell_iterator(const CSVContainer* csv, size_t row_index, const RangeCollection& col_ranges);
		const Cell& operator*();
		cell_iterator& operator++();
	
	private:
		const CSVContainer* csv;
		size_t row_index;
		RangeCollection col_ranges;
	};


	class Row
	{
		friend class CSVContainer;
	
	public:
		Row(const CSVContainer* csv, size_t row_index, const RangeCollection& col_ranges);

		cell_iterator begin();
		cell_iterator end();

	private:
		const CSVContainer* csv;
		size_t row_index;
		RangeCollection col_ranges;
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

		// for (const auto& row : frame)
		//     for (const auto& cell : row)
		//         ...

	private:
		const CSVContainer* csv;
		std::vector<Range> row_ranges;
		std::vector<Range> col_ranges;
	};

private:
	CsvLoadingSettings m_settings;
	std::vector<std::vector<Cell>> m_table;
	std::vector<std::string> m_columnNames;
	size_t m_numRows;
	size_t m_numCols;
};