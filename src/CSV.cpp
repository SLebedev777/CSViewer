#include "CSV.h"
#include <cassert>
#include <algorithm>
#include <iostream>


CSVContainer::cell_iterator::cell_iterator(const CSVContainer::RowView* row_view, typename RangeCollection::chain_iterator iter)
	: row_view(row_view)
	, iter(iter)
{}

const Cell& CSVContainer::cell_iterator::operator*()
{
	const Row& row = *(row_view->row);
	return row[*iter];
}

CSVContainer::RowView::RowView(const Row* row)
	: row(row), col_ranges(Range(0, row->size()))
{}

CSVContainer::RowView::RowView(const Row* row, const RangeCollection& col_ranges)
	: row(row), col_ranges(col_ranges.boundBy(Range(0, row->size())))
{}


CSVContainer::CSVContainer(std::vector<Row>&& data, std::vector<std::string>&& column_names, const std::string& name)
	: m_data(std::move(data))
	, m_columnNames(std::move(column_names))
	, m_settings(CSVLoadingSettings(name))
	, m_numRows(m_data.size())
	, m_numCols(m_data.front().size())
{
	if (!std::all_of(m_data.begin(), m_data.end(), [&](auto& row) { return row.size() == m_data.front().size(); }))
		throw std::logic_error("All rows of CSV data should have the same size!");

	if (m_data.front().size() != m_columnNames.size())
		throw std::logic_error("Size of column names should match number of columns in CSV!");
}

CSVContainer::Frame::Frame(const CSVContainer* csv, size_t row_from, size_t row_to, size_t col_from, size_t col_to)
	: csv(csv)
{
	row_ranges.insert(Range(row_from, (row_to != END) ? row_to : csv->m_numRows ));
	col_ranges.insert(Range(col_from, (col_to != END) ? col_to : csv->m_numCols));
}

CSVContainer::Frame::Frame(const CSVContainer* csv, const Range& range, bool axis)
	: csv(csv)
{
	if (axis == AXIS_ROWS)
	{
		row_ranges.insert(range);
		row_ranges = row_ranges.boundBy(Range(0, csv->m_numRows));
	}
	else
	{
		col_ranges.insert(range);
		col_ranges = col_ranges.boundBy(Range(0, csv->m_numCols));
	}
}

CSVContainer::Frame::Frame(const CSVContainer* csv, const Range& rows, const Range& cols)
	: csv(csv)
{
	row_ranges.insert(rows);
	col_ranges.insert(cols);
}

CSVContainer::Frame::Frame(const CSVContainer* csv, const RangeCollection& ranges, bool axis)
	: csv(csv)
{
	if (axis == AXIS_ROWS)
	{
		row_ranges = ranges.boundBy(Range(0, csv->m_numRows));
	}
	else
	{
		col_ranges = ranges.boundBy(Range(0, csv->m_numCols));
	}
}

CSVContainer::Frame::Frame(const CSVContainer* csv, RangeCollection&& ranges, bool axis)
	: csv(csv)
{
	if (axis == AXIS_ROWS)
	{
		row_ranges = std::move(ranges.boundBy(Range(0, csv->m_numRows)));
	}
	else
	{
		col_ranges = std::move(ranges.boundBy(Range(0, csv->m_numCols)));
	}
}

CSVContainer::Frame::Frame(const CSVContainer* csv, const RangeCollection& row_ranges, const RangeCollection& col_ranges)
	: csv(csv)
	, row_ranges(row_ranges.boundBy(Range(0, csv->m_numRows)))
	, col_ranges(col_ranges.boundBy(Range(0, csv->m_numCols)))
{}

CSVContainer::Frame::Frame(const CSVContainer* csv, RangeCollection&& row_ranges, RangeCollection&& col_ranges)
	: csv(csv)
	, row_ranges(std::move(row_ranges.boundBy(Range(0, csv->m_numRows))))
	, col_ranges(std::move(col_ranges.boundBy(Range(0, csv->m_numCols))))
{}


CSVContainer::row_iterator::row_iterator(const CSVContainer* csv, typename RangeCollection::chain_iterator iter, const RangeCollection* col_ranges)
	: csv(csv)
	, iter(iter)
	, col_ranges(col_ranges)
{
}

CSVContainer::RowView CSVContainer::row_iterator::operator*()
{
	return RowView(&(csv->m_data[*iter]), *col_ranges); 
}

CSVContainer::row_iterator CSVContainer::Frame::begin()
{
	return CSVContainer::row_iterator{ csv, row_ranges.chainBegin(), &col_ranges };
}

CSVContainer::row_iterator CSVContainer::Frame::end()
{
	return CSVContainer::row_iterator{ csv, row_ranges.chainEnd(), &col_ranges };
}


std::ostream& operator<<(std::ostream& os, const CSVContainer::RowView& row_view)
{
	std::copy(row_view.cbegin(), row_view.cend(), std::ostream_iterator<std::string>{os, ", "});
	return os;
}
