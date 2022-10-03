#include "gtest/gtest.h"
#include "Range.h"

TEST(RangeTests, IsEmpty)
{
	Range r{ 5, 5 };
	EXPECT_TRUE(r.isEmpty());
}