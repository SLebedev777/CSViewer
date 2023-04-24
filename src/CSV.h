#ifndef __CSVIEWER_CSV_H__
#define __CSVIEWER_CSV_H__

#include <vector>
#include <string>
#include <stdexcept>
#include <limits>
#include "Range.h"

extern const std::array<std::string, 3> SUPPORTED_INPUT_ENCODINGS;

enum class BadLinesPolicy
{
	BL_RAISE,
	BL_WARN,
	BL_SKIP
};


struct CSVLoadingSettings
{
	explicit CSVLoadingSettings(const std::string& filename,
		const std::string& encoding = "UTF-8",
		char delimiter = ',',
		char quote = '"',
		bool has_header = false,
		BadLinesPolicy bad_lines_policy = BadLinesPolicy::BL_RAISE,
		size_t skip_first_lines = 0
	)
		: filename(filename)
		, encoding(encoding)
		, delimiter(delimiter)
		, quote(quote)
		, has_header(has_header)
		, bad_lines_policy(bad_lines_policy)
		, skip_first_lines(skip_first_lines)
	{}
	
	std::string filename;
	std::string encoding;
	char delimiter;
	char quote;
	bool has_header;
	BadLinesPolicy bad_lines_policy;
	size_t skip_first_lines;
};

bool operator==(const CSVLoadingSettings& left, const CSVLoadingSettings& right);


using Cell = std::string;
using Row = std::vector<Cell>;

class CSVContainer
{
public:
	static const size_t END = std::numeric_limits<size_t>::max();

	CSVContainer(const CSVLoadingSettings& settings);
	CSVContainer(std::vector<Row>&& data, std::vector<std::string>&& column_names, const std::string& name);

	class RowView;

	// iterate over cells of a row, maybe using some range collection
	class cell_iterator
	{
	public:
		using difference_type = std::ptrdiff_t;
		using value_type = Cell;
		using pointer = const value_type*;
		using reference = const value_type&;
		using iterator_category = std::forward_iterator_tag;

		cell_iterator(const RowView* row_view, typename RangeCollection::chain_iterator iter);
		reference operator*();
		cell_iterator& operator++() { ++iter; return *this; };
		bool operator==(const cell_iterator& other) { return (row_view == other.row_view) && (iter == other.iter); }
		bool operator!=(const cell_iterator& other) { return !(*this == other); };
	
	private:
		const RowView* row_view;
		typename RangeCollection::chain_iterator iter;
	};


	// helper class of row read-only view 
	class RowView
	{
		friend class cell_iterator;

	public:
		RowView(const Row* row);
		RowView(const Row* row, const RangeCollection& col_ranges);

		CSVContainer::cell_iterator cbegin() const { return cell_iterator(this, col_ranges.chainBegin()); }
		CSVContainer::cell_iterator cend() const { return cell_iterator(this, col_ranges.chainEnd()); }

		CSVContainer::cell_iterator begin() { return cell_iterator(this, col_ranges.chainBegin()); }
		CSVContainer::cell_iterator end() { return cell_iterator(this, col_ranges.chainEnd()); }

		friend std::ostream& operator<<(std::ostream& os, const RowView& row_view);

	private:
		const Row* row;
		RangeCollection col_ranges;
	};


	// iterate over rows of a frame, maybe using some range collection
	class row_iterator
	{
	public:
		using difference_type = std::ptrdiff_t;
		using value_type = RowView;
		using pointer = const Row*;
		using reference = const value_type&;
		using iterator_category = std::forward_iterator_tag;

		row_iterator(const CSVContainer* csv, typename RangeCollection::chain_iterator iter, const RangeCollection* col_ranges);
		value_type operator*();  // !!! ATTENTION! We return a COPY of RowView r-value object
		row_iterator& operator++() { ++iter; return *this; }
		bool operator==(const row_iterator& other) { return (csv == other.csv) && (iter == other.iter); }
		bool operator!=(const row_iterator& other) { return !(*this == other); }

	private:
		typename RangeCollection::chain_iterator iter;
		const CSVContainer* csv;
		const RangeCollection* col_ranges;
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

		// for (auto& row : frame)
		//     for (const auto& cell : row)
		//         ...
		row_iterator begin();
		row_iterator end();

		const RangeCollection& getRowRanges() const { return row_ranges; }
		const RangeCollection& getColRanges() const { return col_ranges; }
		RowView getColumnNames() const { return RowView(&csv->m_columnNames, col_ranges); }
		size_t getNumRows() const { return row_ranges.totalElements(); }
		size_t getNumCols() const { return col_ranges.totalElements(); }
		std::pair<size_t, size_t> getShape() const { return { getNumRows(), getNumCols() }; }

	private:
		const CSVContainer* csv;
		RangeCollection row_ranges;
		RangeCollection col_ranges;
	};

	void setColumnName(size_t index, const std::string& name);
	std::string getColumnName(size_t index) const;
	size_t findColumnIndex(const std::string& name) const;

private:
	void readCSV(const CSVLoadingSettings& settings);
	void checkSettings(const CSVLoadingSettings& settings) const;

private:
	CSVLoadingSettings m_settings;
	std::vector<Row> m_data;
	Row m_columnNames;
	size_t m_numRows = 0;
	size_t m_numCols = 0;
};

#endif