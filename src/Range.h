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
	size_t lastIncluded() const { return (to > 0) ? (to - 1) : 0; }
	
	bool contains(size_t value) const { return (value >= from) && (value < to); }
	bool intersects(const Range& other) const
	{
		if (contains(other.from) ||
			contains(other.lastIncluded()) ||
			other.contains(from) ||
			other.contains(lastIncluded()))
		{
			return true;
		}
		return false;
	}

	// having range [from;to) iterate over sequence: from, from+1, from+2, ..., to-1
	class const_iterator
	{
	public:
		using difference_type = std::ptrdiff_t;
		using value_type = size_t;
		using pointer = const value_type*;
		using reference = const value_type&;
		using iterator_category = std::forward_iterator_tag;

		explicit const_iterator(size_t value)
			: value(value)
		{}
		reference operator*() const { return value; }
		const_iterator& operator++() { ++value; return *this; }
		bool operator==(const Range::const_iterator& other) const { return value == other.value; }
		bool operator!=(const Range::const_iterator& other) const { return !(*this == other); }

	private:
		size_t value;
	};

	const_iterator cbegin() const { return Range::const_iterator(from); }
	const_iterator cend() const { return Range::const_iterator(to); }
	const_iterator begin() const { return cbegin(); }  // to use range-based for loop
	const_iterator end() const { return cend(); }

	const size_t from;
	const size_t to;
};

bool operator==(const Range& x, const Range& y);
bool operator!=(const Range& x, const Range& y);
bool operator<(const Range& x, const Range& y);


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

	const_iterator cbegin() const { return m_ranges.cbegin(); }
	const_iterator cend() const { return m_ranges.cend(); };

	// having list of ranges [F1;T1), [F2;T2), ..., [Fn;Tn), iterate over sequence F1, F1+1, ..., T1-1, F2, F2+1, ..., T2-1, ..., Fn, Fn+1,...Tn-1
	class chain_iterator
	{
	public:
		using difference_type = std::ptrdiff_t;
		using value_type = size_t;
		using pointer = const value_type*;
		using reference = const value_type&;
		using iterator_category = std::forward_iterator_tag;

		chain_iterator(const RangeCollection::const_iterator& coll_it, const Range::const_iterator& range_it)
			: coll_it(coll_it)
			, range_it(range_it)
		{}
		reference operator*() const { return *range_it; }
		chain_iterator& operator++();
		bool operator==(const chain_iterator& other) const { return (coll_it == other.coll_it) && (range_it == other.range_it); }
		bool operator!=(const chain_iterator& other) const { return !(*this == other); }

	private:
		RangeCollection::const_iterator coll_it;  // iterates over ranges in list
		Range::const_iterator range_it;  // iterates within current range
	};

private:
	bool try_push_back(const Range& range);

private:
	container m_ranges;
	size_t m_totalElements = 0;
};
