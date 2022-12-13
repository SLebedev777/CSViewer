#include "CSV.h"

CSVContainer::Frame::Frame(const CSVContainer* csv, size_t row_from, size_t row_to, size_t col_from, size_t col_to)
	: csv(csv)
{
	row_ranges.insert(Range(row_from, row_to));
	col_ranges.insert(Range(col_from, col_to));
}

CSVContainer::Frame::Frame(const CSVContainer* csv, const Range& range, bool axis)
	: csv(csv)
{
	if (axis == AXIS_ROWS)
	{
		row_ranges.insert(range);
	}
	else
	{
		col_ranges.insert(range);
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
		row_ranges = ranges;
	}
	else
	{
		col_ranges = ranges;
	}
}

CSVContainer::Frame::Frame(const CSVContainer* csv, RangeCollection&& ranges, bool axis)
	: csv(csv)
{
	if (axis == AXIS_ROWS)
	{
		row_ranges = std::move(ranges);
	}
	else
	{
		col_ranges = std::move(ranges);
	}
}

CSVContainer::Frame::Frame(const CSVContainer* csv, const RangeCollection& row_ranges, const RangeCollection& col_ranges)
	: csv(csv)
	, row_ranges(row_ranges)
	, col_ranges(col_ranges)
{}

CSVContainer::Frame::Frame(const CSVContainer* csv, RangeCollection&& row_ranges, RangeCollection&& col_ranges)
	: csv(csv)
	, row_ranges(std::move(row_ranges))
	, col_ranges(std::move(col_ranges))
{}


CSVContainer::row_iterator::row_iterator(const CSVContainer* csv, const RangeCollection* row_ranges, size_t pos)
{}

CSVContainer::row_iterator CSVContainer::Frame::begin()
{
	return CSVContainer::row_iterator{ csv, &row_ranges, *row_ranges.chainBegin() };
}

CSVContainer::row_iterator CSVContainer::Frame::end()
{
	return CSVContainer::row_iterator{ csv, &row_ranges, *row_ranges.chainEnd() };
}

