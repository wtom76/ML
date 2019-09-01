#pragma once

#include <vector>
#include <iostream>
#include <sstream>
#include <algorithm>
#include <memory>

using namespace std;

namespace wtom::ml::Data
{
	//-----------------------------------------------------------------------------------------------------
	// class Normalization
	//-----------------------------------------------------------------------------------------------------
	class Normalization
	{
	public:
		virtual void normalize(vector<double>& series) = 0;
		virtual double restore(double normalized_val) const = 0;
	};

	//-----------------------------------------------------------------------------------------------------
	// class Range
	//-----------------------------------------------------------------------------------------------------
	class Range : private pair<double, double>
	{
	public:
		using pair<double, double>::pair;
		double min() const noexcept { return first; }
		double max() const noexcept { return second; }
		operator double() const noexcept { return second - first; }
	};

	//-----------------------------------------------------------------------------------------------------
	// class RangeNormalization
	//-----------------------------------------------------------------------------------------------------
	class RangeNormalization : public Normalization
	{
		// data
	private:
		Range normal_range_;
		Range original_range_;
		Range current_range_;

		// methods
	private:
	public:
		explicit RangeNormalization(const Range& normal_range) : normal_range_(normal_range) {}
		virtual void normalize(vector<double>& series) override;
		virtual double restore(double normalized_val) const override;
	};
	//-----------------------------------------------------------------------------------------------------
	inline void RangeNormalization::normalize(vector<double>& series)
	{
		const auto min_max = minmax_element(cbegin(series), cend(series));
		original_range_ = make_pair(*min_max.first, *min_max.second);
		if (original_range_.min() == 0. && original_range_.max() == 0.)
		{
			return;
		}
		const double linear_transform = normal_range_ / original_range_;
		const double min = original_range_.min();
		const double normalized_min = normal_range_.min();
		transform(cbegin(series), cend(series), begin(series),
			[min, normalized_min, linear_transform](double val)
			{
				return (val - min) * linear_transform + normalized_min;
			}
		);
		current_range_ = normal_range_;
	}
	//-----------------------------------------------------------------------------------------------------
	inline double RangeNormalization::restore(double normalized_val) const
	{
		return (normalized_val - current_range_.min()) * original_range_ / current_range_
			+ original_range_.min();
	}

	//-----------------------------------------------------------------------------------------------------
	// class Lognormal
	//-----------------------------------------------------------------------------------------------------
	class Lognormal : public Normalization
	{
		// methods
	public:
		virtual void normalize(vector<double>& series) override;
		virtual double restore(double normalized_val) const override;
	};
	//-----------------------------------------------------------------------------------------------------
	inline void Lognormal::normalize(vector<double>& series)
	{
		transform(cbegin(series), cend(series), begin(series),
			[](auto val)
			{
				assert(val > 0.);
				return std::log(val);
			}
		);
	}
	//-----------------------------------------------------------------------------------------------------
	inline double Lognormal::restore(double normalized_val) const
	{
		return std::exp(normalized_val);
	}
}