#include "gtest/gtest.h"
#include "Range.h"
#include <type_traits>


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