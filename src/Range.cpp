#include "Range.h"


bool RangeCollection::insert(const Range& range)
{
	if (try_push_back(range))
		return true;

	auto it = cbegin();
	while (it != cend())
	{
		Range curr = *it;
		if (curr.intersects(range))
			return false;

		if (range < curr)
		{
			m_ranges.insert(it, range);
			m_totalElements += range.size();
			return true;
		}
		else
			++it;
	}
	return false;
}


bool RangeCollection::try_push_back(const Range& range)
{
	if (m_ranges.empty() || m_ranges.back() < range)
	{
		m_ranges.push_back(range);
		m_totalElements += range.size();
		return true;
	}
	return false;
}


RangeCollection::chain_iterator& RangeCollection::chain_iterator::operator++()
{
	++range_it;
	if (range_it == (*coll_it).cend())
	{
		++coll_it;
		range_it = (*coll_it).cbegin();
	}
	return *this;
}
