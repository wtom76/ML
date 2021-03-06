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
		constexpr double min() const noexcept { return first; }
		constexpr double max() const noexcept { return second; }
		constexpr operator double() const noexcept { return second - first; }
		bool is_valid() const noexcept;
	};
	//-----------------------------------------------------------------------------------------------------
	inline bool Range::is_valid() const noexcept
	{
		const double rng = second - first;
		return !std::isnan(rng) && rng >= std::numeric_limits<double>::epsilon();
	}

	//-----------------------------------------------------------------------------------------------------
	inline Range min_max(const std::vector<double>& series) noexcept
	{
		assert(!series.empty());
		double min_val{std::numeric_limits<double>::max()};
		double max_val{-std::numeric_limits<double>::max()};
		for (const double val : series)
		{
			if (std::isnan(val))
			{
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
		return {min_val, max_val};
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