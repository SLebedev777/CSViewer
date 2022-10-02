#pragma once

#include <list>
#include <stdexcept>


// constant interval [from; to)
struct Range
{
	explicit Range(size_t from, size_t to)
		: from(from)
		, to(to)
	{
		if (to < from)
			throw std::logic_error("Range error: to < from");
	}

	size_t size() const { return to - from; }
	bool isEmpty() const { return from == to; }

	bool contains(size_t value) const { return (value >= from) && (value < to); }
	bool intersects(const Range& other) const
	{
		if (contains(other.from) ||
			contains(other.to) ||
			other.contains(from) ||
			other.contains(to))
		{
			return true;
		}
		else
			return false;
	}

	const size_t from;
	const size_t to;
};

bool operator==(const Range& x, const Range& y)
{
	return (x.from == y.from) && (x.to == y.to);
}

bool operator!=(const Range& x, const Range& y)
{
	return !(x == y);
}


// ordered collection of ranges, arranged in sorted non-descending order
class RangeCollection
{
public:

	using container = std::list<Range>;
	using const_iterator = container::const_iterator;

	RangeCollection() = default;

	bool insert(const Range& range);

	size_t size() const { return m_ranges.size(); }
	size_t totalElements() const { return m_totalElements; }

	const_iterator cbegin();
	const_iterator cend();

private:
	bool check(const Range& range);
	void sort();
	void update();

private:
	container m_ranges;
	size_t m_totalElements = 0;
};
