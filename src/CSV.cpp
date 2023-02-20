#include "CSV.h"
#include "IConvConverter.h"
#include <cassert>
#include <algorithm>
#include <iostream>
#include <fstream>
#include <array>

static const std::array<std::string, 3> SUPPORTED_INPUT_ENCODINGS = { "UTF-8", "CP1251", "CP866" };

namespace
{
	template <typename Iter, typename OutIter, typename T, typename SliceFunc>
	void split(Iter first, Iter last, OutIter out, const T& sep, const T& quote, SliceFunc slice_func,
		bool remove_consecutive = false)
	{
		while (first != last)
		{
			Iter slice_end;
			if (*first == quote)
			{
				auto quote_end = std::next(first);
				do
				{
					quote_end = std::find(quote_end, last, quote);
					if (quote_end == last)
						throw std::logic_error("wrong quotes error");
					++quote_end;
				} while (quote_end != last && *quote_end != sep);
				slice_end = quote_end;
			}
			else
			{
				slice_end = std::find(first, last, sep);
			}
			if ((first == slice_end) && remove_consecutive)
			{
				++first;
				continue;
			}
			*out = slice_func(first, slice_end);
			if (slice_end == last)
				return;
			++out;
			first = ++slice_end;
			if (first == last)
				*out = slice_func(slice_end, first);
		}
	}

	// overloading for char separator (for splitting strings)
	template <typename Iter, typename OutIter, typename SliceFunc>
	void split(Iter first, Iter last, OutIter out, const char& sep, const char& quote, SliceFunc slice_func)
	{
		bool remove_consecutive = sep == ' ';
		split(first, last, out, sep, quote, slice_func, remove_consecutive);
	}
}


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

CSVContainer::CSVContainer(const CSVLoadingSettings& settings)
	: m_settings(settings)
{
	checkSettings(settings);
	readCSV(settings);
}

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
	row_ranges.insert(Range(row_from, (row_to != END) ? row_to : csv->m_numRows));
	row_ranges = row_ranges.boundBy(Range(0, csv->m_numRows));
	col_ranges.insert(Range(col_from, (col_to != END) ? col_to : csv->m_numCols));
	col_ranges = col_ranges.boundBy(Range(0, csv->m_numCols));
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
	row_ranges = row_ranges.boundBy(Range(0, csv->m_numRows));
	col_ranges.insert(cols);
	col_ranges = col_ranges.boundBy(Range(0, csv->m_numCols));
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

void CSVContainer::checkSettings(const CSVLoadingSettings& settings) const
{
	if (std::find(SUPPORTED_INPUT_ENCODINGS.begin(), SUPPORTED_INPUT_ENCODINGS.end(), settings.encoding) ==
		SUPPORTED_INPUT_ENCODINGS.end())
	{
		throw std::logic_error("Unsupported input encoding: " + settings.encoding);
	}

	if (settings.delimiter == settings.quote)
	{
		throw std::logic_error("Delimiter can't be equal to quote symbol!");
	}
}

void CSVContainer::readCSV(const CSVLoadingSettings& settings)
{
	if (!m_data.empty())
		throw std::runtime_error("CSVContainer data not empty before loading new CSV. Overwriting not supported.");

	std::ifstream file(settings.filename);
	std::string line;
	bool is_first_line = true;
	size_t i = 0;

	auto conv = utils::IConvConverter(settings.encoding, "UTF-8");

	while (std::getline(file, line))
	{
		try
		{
			std::string converted_line = conv.convert(line);

			Row row;
			split(converted_line.begin(), converted_line.end(), std::back_inserter(row), settings.delimiter, settings.quote,
				[](auto first, auto last) {
					return std::string(first, last);
				}
			);
			if (is_first_line)
			{
				m_numCols = row.size();
				is_first_line = false;

				if (settings.has_header)
				{
					m_columnNames = row;
					continue;
				}
			}
			if (row.size() != m_numCols)
			{
				throw std::runtime_error("wrong number of cells, should be " + std::to_string(m_numCols));
			}
			m_data.push_back(row);
			++i;
		}
		catch (std::exception& ex)
		{
			throw std::runtime_error("CSV reading error, line " + std::to_string(i) + ": " + ex.what());
		}
		catch (...)
		{
			throw std::runtime_error("CSV reading error, line " + std::to_string(i));
		}
	}
	m_numRows = m_data.size();

	if (m_columnNames.empty())
	{
		for (size_t j = 0; j < m_numCols; j++)
			m_columnNames.push_back(std::to_string(j));
	}

	file.close();
}