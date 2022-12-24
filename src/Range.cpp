#include "Range.h"
#include <sstream>

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

std::ostream& operator<<(std::ostream& os, const Range& range)
{
	os << "[" << range.from << ", " << range.to << ")";
	return os;
}

std::string Range::to_string() const
{
	std::ostringstream oss;
	oss << *this;
	return oss.str();
}

RangeCollection::RangeCollection(const Range& range)
{
	insert(range);
}

RangeCollection::RangeCollection(std::initializer_list<Range> range_list)
{
	for (auto range : range_list)
	{
		if (!insert(range))
			throw std::logic_error("Failed to construct RangeCollection! This Range in initializer list intersects with others: " + range.to_string());
	}
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
