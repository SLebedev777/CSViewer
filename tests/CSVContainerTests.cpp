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

	std::vector<std::string> range_collection_to_str_vector(const RangeCollection& rc, const std::string& postfix = "")
	{
		std::vector<std::string> result;
		std::transform(rc.chainBegin(), rc.chainEnd(), std::back_inserter(result),
			[&postfix](const auto& element) { return std::to_string(element) + postfix; });
		return result;
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

TEST(CSVContainerTests, CSVContainerFrameRangeCollectionByRowsNeedsBound)
{
	const size_t NUM_ROWS = 15;
	const size_t NUM_COLS = 4;
	auto csv = make_csv_container(NUM_ROWS, NUM_COLS);

	RangeCollection rc;
	rc.insert(Range(2, 5));
	rc.insert(Range(7, 9));
	rc.insert(Range(12, 20));  // this range exceeds frame shape by rows, so we need to bound it
	RangeCollection rc_bound = rc.boundBy(Range(0, NUM_ROWS));
	
	CSVContainer::Frame frame(&csv, rc);  // feed unbound rc to frame - should be bound by frame ctor
	size_t i = 0;
	for (auto row_it = frame.begin(); row_it != frame.end(); ++row_it, ++i)
	{
		EXPECT_NO_THROW(*row_it); // test dereference doesn't throw because of "out of range" exception
	}
	EXPECT_EQ(rc_bound.totalElements(), i);
}
