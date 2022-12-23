#include "gtest/gtest.h"
#include "CSV.h"
#include <algorithm>


namespace
{
	CSVContainer make_csv_container(size_t n_rows, size_t n_cols)
	{
		std::vector<Row> data;
		for (size_t i = 0; i < n_rows; ++i)
		{
			auto si = std::to_string(i);
			Row row(n_cols);
			std::generate(row.begin(), row.end(), [&si, j = 0]() mutable { return si + "_" + std::to_string(j++); });
			data.push_back(std::move(row));
		}

		std::vector<std::string> column_names(n_cols);
		std::generate(column_names.begin(), column_names.end(), [j = 0]() mutable { return "C" + std::to_string(j++); });
		return CSVContainer(std::move(data), std::move(column_names), "test_" + std::to_string(n_rows) + "_" + std::to_string(n_cols));
	}
}

TEST(CSVContainerTests, CSVContainerMakeContainer)
{
	auto csv = make_csv_container(5, 3);
	EXPECT_TRUE(true);
}

TEST(CSVContainerTests, CSVContainerFrame1RowIterator1)
{
	auto csv = make_csv_container(3, 4);
	CSVContainer::Frame frame(&csv);
	std::vector<size_t> v;
	for (CSVContainer::row_iterator row_it = frame.begin(); row_it != frame.end(); ++row_it)
	{
		std::cout << *row_it << std::endl;
	}
	for (const auto& row : frame)
	{
		std::cout << row << std::endl;
	}
}

TEST(CSVContainerTests, CSVContainerFrameSingleRowRange)
{
	auto csv = make_csv_container(10, 4);
	CSVContainer::Frame frame(&csv, Range(3, 7));
	for (const auto& row : frame)
	{
		std::cout << row << std::endl;
	}
}

TEST(CSVContainerTests, CSVContainerFrameRangeCollectionByRows)
{
	auto csv = make_csv_container(15, 4);
	RangeCollection rc;
	rc.insert(Range(2, 5));
	rc.insert(Range(7, 9));
	rc.insert(Range(12, 13));
	CSVContainer::Frame frame(&csv, rc);
	for (const auto& row : frame)
	{
		std::cout << row << std::endl;
	}
}
