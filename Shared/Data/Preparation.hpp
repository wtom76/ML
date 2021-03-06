#pragma once
#include <vector>
#include <cmath>
#include <iterator>

namespace wtom::ml::data
{
	inline void fill_missing_forward(std::vector<double>& series, size_t max_tolerated_gap) noexcept
	{
		auto val_i = series.begin();
		const auto val_e = series.cend();
		std::vector<double>::const_iterator valid_val_i;
		for (; val_i != val_e && std::isnan(*val_i); ++val_i)
		{}
		while (val_i != val_e)
		{
			for (; val_i != val_e && !std::isnan(*val_i); ++val_i)
			{}
			if (val_i != series.begin())
			{
				valid_val_i = std::prev(val_i);
				assert(!std::isnan(*valid_val_i));
			}
			for (size_t gap = 0; val_i != val_e && std::isnan(*val_i) && gap < max_tolerated_gap; ++val_i, ++gap)
			{
				*val_i = *valid_val_i;
			}
			for (; val_i != val_e && std::isnan(*val_i); ++val_i)
			{}
		}
	}
}