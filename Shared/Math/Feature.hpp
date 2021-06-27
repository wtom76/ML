#pragma once
#include <vector>
#include <array>
#include <cmath>

using std::ptrdiff_t;

namespace wtom::ml::math
{
	//---------------------------------------------------------------------------------------------------------
	template <class Container>
	constexpr bool safe_advance(typename Container::const_iterator& iter, const Container& cont, ptrdiff_t offset)
	{
		if (offset > 0 && std::distance(iter, std::cend(cont)) < offset)
		{
			return false;
		}
		if (offset < 0 && std::distance(std::cbegin(cont), iter) < -offset)
		{
			return false;
		}
		std::advance(iter, offset);
		return true;
	}
	//---------------------------------------------------------------------------------------------------------
	/// Calculates difference between current value and value \param period ago
	/// \param future_shift is a shift of dest in relation to source. To make target which contains changes that
	/// will be on a next day, set future_shift = 1
	inline void make_delta(const std::vector<double>& src, std::vector<double>& dst, size_t max_tolerated_gap, ptrdiff_t period = 1, ptrdiff_t future_shift = 0)
	{
		assert(period > 0);
		assert(future_shift >= 0);

		dst.resize(src.size(), std::numeric_limits<double>::quiet_NaN());
		auto src_i = std::cbegin(src);
		auto src_e = std::cend(src);
		// 1.
		if (!safe_advance(src_i, src, period))
		{
			return;
		}
		// 2.
		while (src_i != src_e)
		{
			for (; src_i != src_e && std::isnan(*src_i); ++src_i)
			{}
			if (src_i == src_e)
			{
				return;
			}
			size_t cur_gap = 0;
			auto prev_src_ri = std::make_reverse_iterator(src_i);
			std::advance(prev_src_ri, period - 1);
			while (prev_src_ri != std::crend(src) && cur_gap < max_tolerated_gap && std::isnan(*prev_src_ri))
			{
				++cur_gap;
				++prev_src_ri;
			}
			if (prev_src_ri != std::crend(src) && !std::isnan(*prev_src_ri))
			{
				auto dst_i = std::next(std::begin(dst), std::distance(std::cbegin(src), src_i));
				if (safe_advance(dst_i, dst, -future_shift))
				{
					*dst_i = *src_i - *prev_src_ri;
				}
			}
			++src_i;
		}
	}
	//---------------------------------------------------------------------------------------------------------
	inline bool equal_with_nan(const std::vector<double>& left, const std::vector<double>& right)
	{
		if (left.size() != right.size())
		{
			return false;
		}
		auto left_i = left.cbegin();
		auto left_e = left.cend();
		auto right_i = right.cbegin();
		for (; left_i != left_e; ++left_i, ++right_i)
		{
			if (std::isnan(*left_i) && std::isnan(*right_i))
			{
				continue;
			}
			if (*left_i != *right_i)
			{
				return false;
			}
		}
		return true;
	}
	//---------------------------------------------------------------------------------------------------------
	inline void test_make_delta()
	{
		constexpr double nan_ = std::numeric_limits<double>::quiet_NaN();
		// 1.
		{
			std::vector<double> dst;
			make_delta({ 1, 2, 3, 4, 5 }, dst, 0, 1, 0);
			assert(equal_with_nan({ nan_, 1, 1, 1, 1 }, dst));
		}
		// 2.
		{
			std::vector<double> dst;
			make_delta({ 1, 2, nan_, nan_, 3, 4, 5 }, dst, 2, 1, 0);
			assert(equal_with_nan({ nan_, 1, nan_, nan_, 1, 1, 1 }, dst));
		}
		// 3.
		{
			std::vector<double> dst;
			make_delta({ 1, 2, nan_, nan_, nan_, 3, 4, 5 }, dst, 2, 1, 0);
			assert(equal_with_nan({ nan_, 1, nan_, nan_, nan_, nan_, 1, 1 }, dst));
		}
		// 4.
		{
			std::vector<double> dst;
			make_delta({ 1, 2, nan_, nan_, nan_, 3, 4, 5 }, dst, 2, 1, 1);
			assert(equal_with_nan({ 1, nan_, nan_, nan_, nan_, 1, 1, nan_}, dst));
		}
		// 5.
		{
			std::vector<double> dst;
			make_delta({ 1, 2, nan_, nan_, nan_, 3, 4, 5 }, dst, 2, 3, 0);
			assert(equal_with_nan({ nan_, nan_, nan_, nan_, nan_, 1, 2, nan_ }, dst));
		}
		// 6.
		{
			std::vector<double> dst;
			make_delta({ 1, 2, nan_, nan_, nan_, 3, 4, 5 }, dst, 3, 3, 0);
			assert(equal_with_nan({ nan_, nan_, nan_, nan_, nan_, 1, 2, 3 }, dst));
		}
	}
	//---------------------------------------------------------------------------------------------------------
	/// will be on a next day, set future_shift = 1
	/// Filtered by threshold next_non_nan_close - prev_close (prev means that current figure is shifted one step backward)
	inline void make_win_loss_close(const std::vector<double>& src, std::vector<double>& dst, double threshold, ptrdiff_t future_shift = 0)
	{
		assert(future_shift >= 0);

		dst.resize(src.size(), std::numeric_limits<double>::quiet_NaN());
		auto src_i = std::cbegin(src);
		auto dst_i = std::begin(dst);
		auto src_e = std::cend(src);
		// 1.
		if (!safe_advance(src_i, src, future_shift))
		{
			return;
		}
		// 2.
		for (; src_i != src_e; ++src_i, ++dst_i)
		{
			if (std::isnan(*src_i))
			{
				continue;
			}
			const double prev_src = *src_i;
			auto fwd_src_i = src_i;
			++fwd_src_i;
			for (; fwd_src_i != src_e; ++fwd_src_i)
			{
				if (std::isnan(*fwd_src_i))
				{
					continue;
				}
				const double delta = *fwd_src_i - prev_src;
				if (delta >= threshold)
				{
					*dst_i = delta;
					break;
				}
				else if (delta <= -threshold)
				{
					*dst_i = delta;
					break;
				}
			}
			if (fwd_src_i == src_e)
			{
				*dst_i = 0.;
			}
		}
	}
	//---------------------------------------------------------------------------------------------------------
	/// will be on a next day, set future_shift = 1
	/// Filtered by threshold c > o ? h - o : o - l : 0.
	inline void make_target_ohlc(
		const std::array<const std::vector<double>*, 4>& ohlc,
		std::vector<double>& dst,
		double threshold,
		ptrdiff_t future_shift = 0
	)
	{
		assert(future_shift >= 0);
		assert(ohlc[0]->size() == ohlc[1]->size());
		assert(ohlc[0]->size() == ohlc[2]->size());
		assert(ohlc[0]->size() == ohlc[3]->size());

		dst.resize(ohlc[0]->size(), std::numeric_limits<double>::quiet_NaN());
		auto src_o_i = std::cbegin(*ohlc[0]);
		auto src_h_i = std::cbegin(*ohlc[1]);
		auto src_l_i = std::cbegin(*ohlc[2]);
		auto src_c_i = std::cbegin(*ohlc[3]);
		auto dst_i = std::begin(dst);
		auto src_o_e = std::cend(*ohlc[0]);
		// 1.
		if (!safe_advance(src_o_i, *ohlc[0], future_shift) ||
			!safe_advance(src_h_i, *ohlc[1], future_shift) ||
			!safe_advance(src_l_i, *ohlc[2], future_shift) ||
			!safe_advance(src_c_i, *ohlc[3], future_shift))
		{
			return;
		}
		// 2.
		for (; src_o_i != src_o_e; ++src_o_i, ++src_h_i, ++src_l_i, ++src_c_i, ++dst_i)
		{
			if (std::isnan(*src_o_i) || std::isnan(*src_h_i) || std::isnan(*src_l_i) || std::isnan(*src_c_i))
			{
				continue;
			}
			const double delta = *src_c_i > *src_o_i ? *src_h_i - *src_o_i : (*src_c_i < *src_o_i ? *src_o_i - *src_l_i : 0.);
			if (delta >= threshold)
			{
				*dst_i = delta;
			}
			else if (delta <= -threshold)
			{
				*dst_i = delta;
			}
			else
			{
				*dst_i = 0.;
			}
		}
	}
}
