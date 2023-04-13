#pragma once
#include <string>
#include <algorithm>

namespace utils
{
	template <typename Iter, typename OutIter, typename T, typename SliceFunc>
	void split(Iter first, Iter last, OutIter out, const T& sep, const T& quote, SliceFunc slice_func,
		bool remove_consecutive)
	{
		while (first != last)
		{
			Iter slice_end;
			if (*first == quote)
			{
				auto quote_end = std::next(first);
				do
				{
					quote_end = std::find(quote_end, last, quote);  // find first next quote
					if (quote_end == last)
						throw std::logic_error("wrong quotes error");
					++quote_end;
					if (quote_end == last)
						break;
					if (*quote_end == quote)  // double quote encountered - should skip it
					{
						++quote_end;
						if (quote_end == last)
							throw std::logic_error("wrong double quotes error");
						if (*quote_end == sep) // can't be sep after double quotes: sequence "", is wrong (but """, is ok)
							++quote_end;
					}
				} while (quote_end != last && *quote_end != sep);
				slice_end = quote_end;
			}
			else
			{
				slice_end = std::find(first, last, sep);
				if (std::find(first, slice_end, quote) < slice_end)
					throw std::logic_error("wrong quotes error");
			}
			if ((first == slice_end) && remove_consecutive)
			{
				++first;
				continue;
			}
			*out = slice_func(first, slice_end);
			if (slice_end == last)
				return;
			++out;
			first = ++slice_end;
			if (first == last)
				*out = slice_func(slice_end, first);
		}
	}

	// specialization for char separator (for splitting strings)
	template <typename Iter, typename OutIter, typename SliceFunc>
	void split(Iter first, Iter last, OutIter out, const char& sep, const char& quote, SliceFunc slice_func)
	{
		bool remove_consecutive = sep == ' ';
		utils::split(first, last, out, sep, quote, slice_func, remove_consecutive);
	}

}