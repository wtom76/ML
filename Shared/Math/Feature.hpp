#pragma once
#include <vector>
#include <cmath>

namespace wtom::ml::math
{
	inline void make_target_next_change(const std::vector<double>& src, std::vector<double>& dst, size_t max_tolerated_gap)
	{
		dst.resize(src.size(), std::numeric_limits<double>::quiet_NaN());
		auto src_i = cbegin(src);
		auto src_e = cend(src);
		auto dst_i = begin(dst);
		for (; src_i != src_e && std::isnan(*src_i); ++src_i, ++dst_i)
		{}
		double prev_src = *src_i++;
		assert(!std::isnan(prev_src));
		size_t cur_gap = 0;
		for (; src_i != src_e; ++src_i, ++dst_i)
		{
			if (std::isnan(*src_i))
			{
				++cur_gap;
				continue;
			}
			if (cur_gap)
			{
				if (cur_gap > max_tolerated_gap)
				{
					prev_src = *src_i;
					cur_gap = 0;
					continue;
				}
				cur_gap = 0;
			}

			*dst_i = *src_i - prev_src;
			prev_src = *src_i;
		}
	}
}
