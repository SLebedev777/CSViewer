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

TEST(RangeCollectionTests, RangeCollectionCtorFromRange)
{
	Range range(4, 8);
	RangeCollection rc(range);
	EXPECT_EQ(1, rc.size());
	EXPECT_EQ(*rc.cbegin(), range);
}

TEST(RangeCollectionTests, RangeCollectionCtorFromInitializerList)
{
	Range r1(4, 8), r2(15, 17), r3(20, 23);
	RangeCollection rc({r1, r2, r3});
	EXPECT_EQ(3, rc.size());
	std::vector<size_t> result;
	std::copy(rc.chainBegin(), rc.chainEnd(), std::back_inserter(result));
	std::vector<size_t> expected{ 4, 5, 6, 7, 15, 16, 20, 21, 22 };
	EXPECT_EQ(expected, result);
}

TEST(RangeCollectionTests, RangeCollectionCtorFromBadInitializerList)
{
	Range r1(4, 8), r2(15, 17), r3(5, 20);  // 3rd range intersects others, can't create RangeCollection from these ranges
	EXPECT_THROW(RangeCollection({ r1, r2, r3 }), std::logic_error);
}

TEST(RangeCollectionTests, RangeCollectionEqualityOperator)
{
	{
		RangeCollection rc1;
		RangeCollection rc2(rc1);
		EXPECT_TRUE(rc1 == rc2);
	}
	{
		RangeCollection rc1({ Range(4, 8), Range(15, 17), Range(20, 25) });
		RangeCollection rc2(rc1);
		EXPECT_TRUE(rc1 == rc2);
	}
	{
		RangeCollection rc1;
		RangeCollection rc2(Range(5, 10));
		EXPECT_TRUE(rc1 != rc2);
	}
	{
		RangeCollection rc1({ Range(4, 8), Range(15, 17), Range(20, 25) });
		RangeCollection rc2({ Range(4, 8), Range(13, 17), Range(20, 25) });
		EXPECT_TRUE(rc1 != rc2);
	}

}

TEST(RangeCollectionTests, RangeCollectionBoundBy)
{
	// limits contain all RangeCollection of single Range
	{
		RangeCollection rc({ Range(5, 10) });
		Range limits(2, 15);
		RangeCollection result = rc.boundBy(limits);
		RangeCollection expected = rc;
		EXPECT_EQ(expected, result);
	}
	// limits contain all RangeCollection of many Ranges
	{
		RangeCollection rc({ Range(5, 10), Range(12, 14), Range(20, 22) });
		Range limits(5, 22);
		RangeCollection result = rc.boundBy(limits);
		RangeCollection expected = rc;
		EXPECT_EQ(expected, result);
	}
	// all(limits < r for r in RangeCollection) == true
	{
		RangeCollection rc({ Range(5, 10), Range(12, 14), Range(20, 22) });
		Range limits(0, 5);
		RangeCollection result = rc.boundBy(limits);
		RangeCollection expected; // empty
		EXPECT_EQ(expected, result);
	}
	// all(limits > r for r in RangeCollection) == true
	{
		RangeCollection rc({ Range(5, 10), Range(12, 14), Range(20, 22) });
		Range limits(22, 25);
		RangeCollection result = rc.boundBy(limits);
		RangeCollection expected; // empty
		EXPECT_EQ(expected, result);
	}
	// Range contains limits
	{
		RangeCollection rc({ Range(0, 4), Range(5, 10), Range(12, 14), Range(20, 22) });
		Range limits(7, 9);
		RangeCollection result = rc.boundBy(limits);
		RangeCollection expected(limits);
		EXPECT_EQ(expected, result);
	}
	// limits fully cuts some 1 range
	{
		Range r1(0, 4), r2(5, 10), r3(20, 22);
		RangeCollection rc({ r1, r2, r3 });
		Range limits(0, r3.from);
		RangeCollection result = rc.boundBy(limits);
		RangeCollection expected({r1, r2});
		EXPECT_EQ(expected, result);
	}
	// limits partially cuts some 1 range
	{
		Range r1(0, 4), r2(5, 10), r3(20, 22);
		RangeCollection rc({ r1, r2, r3 });
		Range limits(0, r3.from + 1);
		RangeCollection result = rc.boundBy(limits);
		RangeCollection expected({ r1, r2, Range(r3.from, r3.from + 1) });
		EXPECT_EQ(expected, result);
	}
	// limits partially cuts some 1 range from left
	{
		Range r1(0, 10), r2(20, 30);
		RangeCollection rc({ r1, r2 });
		Range limits(0, 25);
		RangeCollection result = rc.boundBy(limits);
		RangeCollection expected({ r1, Range(20, 25) });
		EXPECT_EQ(expected, result);
	}
	// limits partially cuts some 1 range from right
	{
		Range r1(0, 10), r2(20, 30);
		RangeCollection rc({ r1, r2 });
		Range limits(5, 50);
		RangeCollection result = rc.boundBy(limits);
		RangeCollection expected({ Range(5, 10), Range(20, 30) });
		EXPECT_EQ(expected, result);
	}
	// limits partially cuts 2 ranges
	{
		Range r1(0, 10), r2(20, 30), r3(40, 50);
		RangeCollection rc({ r1, r2, r3 });
		Range limits(5, 45);
		RangeCollection result = rc.boundBy(limits);
		RangeCollection expected({ Range(5, 10), r2, Range(40, 45) });
		EXPECT_EQ(expected, result);
	}

}