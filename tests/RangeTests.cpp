#include "gtest/gtest.h"
#include "Range.h"
#include <algorithm>
#include <numeric>


TEST(RangeTests, RangeIsEmpty)
{
	Range r{ 5, 5 };
	EXPECT_TRUE(r.isEmpty());
}

TEST(RangeTests, RangeCtorInverted)
{
	EXPECT_THROW(Range(15, 10), std::logic_error);
}

TEST(RangeTests, RangeContains)
{
	Range r{ 10, 15 };
	EXPECT_FALSE(r.contains(r.from - 1));
	EXPECT_TRUE(r.contains(r.from));
	EXPECT_TRUE(r.contains(r.to - 1));
	EXPECT_FALSE(r.contains(r.to));

	Range empty{ 10, 10 };
	EXPECT_FALSE(empty.contains(empty.from));
	EXPECT_FALSE(empty.contains(empty.to));
}

TEST(RangeTests, RangeIntersects)
{
	Range r{ 10, 15 };
	EXPECT_FALSE(Range(5, 9).intersects(r));
	EXPECT_FALSE(Range(5, 10).intersects(r));
	EXPECT_TRUE(Range(5, 20).intersects(r));
	EXPECT_TRUE(r.intersects(Range(5, 20)));
	EXPECT_TRUE(Range(r).intersects(r));
	EXPECT_TRUE(Range(5, r.lastIncluded()).intersects(r));
	EXPECT_TRUE(Range(r.lastIncluded(), 20).intersects(r));
	EXPECT_FALSE(Range(r.to, r.to * 2).intersects(r));
	EXPECT_FALSE(r.intersects(Range(r.to, r.to * 2)));
	EXPECT_TRUE(Range(r.from, r.from).intersects(r));
	EXPECT_TRUE(Range(r.lastIncluded(), r.lastIncluded()).intersects(r));
	EXPECT_FALSE(Range(r.to + 1, r.to + 1).intersects(r));
}

TEST(RangeTests, RangeIterator)
{
	Range r{ 10, 15 };
	EXPECT_EQ(*(r.cbegin()), r.from);
	EXPECT_EQ(*(r.cend()), r.to);
	auto it = r.cbegin();
	++it;
	EXPECT_EQ(*it, r.from + 1);
}

TEST(RangeTests, RangeOperators)
{
	Range r{ 10, 15 };
	EXPECT_TRUE(r < Range(20, 30));
	EXPECT_TRUE(r < Range(r.to, r.to + 1));
	EXPECT_FALSE(r < Range(12, 12));
	EXPECT_TRUE(r == Range(10, 15));
}

TEST(RangeCollectionTests, RangeCollectionChainIterator)
{
	{
		RangeCollection rc;
		rc.insert(Range(10, 15));
		rc.insert(Range(20, 23));
		rc.insert(Range(30, 31));
	
		std::vector<size_t> expected{ 10, 11, 12, 13, 14, 20, 21, 22, 30 };
		std::vector<size_t> result;
		std::copy(rc.chainBegin(), rc.chainEnd(), std::back_inserter(result));
		EXPECT_EQ(expected, result);
	}
	{
		RangeCollection rc;
		rc.insert(Range(1, 2));
		rc.insert(Range(2, 5));
		rc.insert(Range(6, 6));
		rc.insert(Range(10, 13));
		rc.insert(Range(20, 20));

		std::vector<size_t> expected{ 1, 2, 3, 4, 6, 10, 11, 12, 20 };
		std::vector<size_t> result;
		std::copy(rc.chainBegin(), rc.chainEnd(), std::back_inserter(result));
		EXPECT_EQ(expected, result);
	}
}

TEST(RangeCollectionTests, RangeCollectionEmpty)
{
	RangeCollection rc;
	size_t i = 0;
	for (auto it = rc.chainBegin(); it != rc.chainEnd(); ++it)
	{
		*it;
		++i;
	}
	EXPECT_EQ(0, i);

	auto rc2 = RangeCollection();
	EXPECT_EQ(0, rc2.size());
}

TEST(RangeCollectionTests, RangeCollectionChainIterator2)
{
	{
		RangeCollection rc;
		const size_t N = 9;
		rc.insert(Range(0, N));
		std::vector<int> result;
		for (auto it = rc.chainBegin(); it != rc.chainEnd(); ++it)
		{
			result.push_back(*it);
		}
		std::vector<int> expected(N);
		std::iota(expected.begin(), expected.end(), 0);
		EXPECT_EQ(expected, result);
	}
}