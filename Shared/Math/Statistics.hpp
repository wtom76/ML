#pragma once

#include <vector>
#include <algorithm>

namespace wtom::ml::math
{
	//-----------------------------------------------------------------------------------------------------
	// class Range
	//-----------------------------------------------------------------------------------------------------
	class Range : private std::pair<double, double>
	{
	public:
		using pair<double, double>::pair;
		double min() const noexcept { return first; }
		double max() const noexcept { return second; }
		operator double() const noexcept { return second - first; }
	};

	//-----------------------------------------------------------------------------------------------------
	inline Range min_max(const std::vector<double>& series) noexcept
	{
		const auto min_max = std::minmax_element(std::cbegin(series), std::cend(series));
		return {*min_max.first, *min_max.second};
	}
	//-----------------------------------------------------------------------------------------------------
	inline Range min_max_nan(const std::vector<double>& series, size_t& nan_count) noexcept
	{
		assert(!series.empty());
		size_t nan_count_local = 0;
		double min_val{std::numeric_limits<double>::max()};
		double max_val{-std::numeric_limits<double>::max()};
		for (const double val : series)
		{
			if (std::isnan(val))
			{
				++nan_count_local;
				continue;
			}
			if (val > max_val)
			{
				max_val = val;
			}
			if (val < min_val)
			{
				min_val = val;
			}
		}
		nan_count = nan_count_local;
		return {min_val, max_val};
	}
}