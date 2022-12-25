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

bool operator>(const Range& x, const Range& y)
{
	return y < x;
}

bool Range::contains(const Range& other) const
{
	return (from <= other.from) && (to >= other.to);
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

RangeCollection RangeCollection::boundBy(const Range& limits)
{
	RangeCollection result;
	for (auto range_it = cbegin(); range_it != cend(); ++range_it)
	{
		const Range& r = *range_it;
		if (r < limits)
			continue;
		else if (r > limits)
			break;
		else if (limits.contains(r))
			result.insert(r);
		else if (r.contains(limits))
			result.insert(limits);
		else
		{
			// r and limits overlap
			size_t new_from = std::max(r.from, limits.from);
			size_t new_to = std::min(r.to, limits.to);
			result.insert(Range(new_from, new_to));
		}
	}
	return result;
}

bool operator==(const RangeCollection& left, const RangeCollection& right)
{
	if (left.size() != right.size())
		return false;

	auto left_it = left.cbegin();
	auto right_it = right.cbegin();
	for (; left_it != left.cend(), right_it != right.cend(); ++left_it, ++right_it)
	{
		if (*left_it != *right_it)
			return false;
	}
	return true;
}

bool operator!=(const RangeCollection& left, const RangeCollection& right)
{
	return !(left == right);
}