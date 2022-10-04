#include "Range.h"


bool operator==(const Range& x, const Range& y)
{
	return (x.from == y.from) && (x.to == y.to);
}

bool operator!=(const Range& x, const Range& y)
{
	return !(x == y);
}

bool operator<(const Range& x, const Range& y)
{
	return x.to <= y.from;
}

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
	if (coll_it == coll_it_end)
	{
		range_it = Range::const_iterator(0);
		return *this;
	}

	++range_it;
	if ((range_it == (*coll_it).cend()) || coll_it->isEmpty())
	{
		++coll_it;
		if (coll_it == coll_it_end)
		{
			range_it = Range::const_iterator(0);
			return *this;
		}

		range_it = coll_it->begin();
	}

	return *this;
}
