#pragma once

#include <vector>
#include <algorithm>
#include <Shared/Math/Statistics.hpp>

namespace wtom::ml::math
{
	//-----------------------------------------------------------------------------------------------------
	enum class NormalizationMethod : int
	{
		Null, Sigmoid, Tanh
	};

	//-----------------------------------------------------------------------------------------------------
	inline void normalize_range(Range original_range, Range normal_range, std::vector<double>& series)
	{
		assert(normal_range.is_valid());
		if (!original_range.is_valid())
		{
			throw std::runtime_error("normalize_range: original range is nan or less than epsilon");
		}
		const double linear_transform = normal_range / original_range;
		const double min = original_range.min();
		const double normal_min = normal_range.min();
		transform(cbegin(series), cend(series), begin(series),
			[min, normal_min, linear_transform](double val)
			{
				return (val - min) * linear_transform + normal_min;
			}
		);
	}
	//-----------------------------------------------------------------------------------------------------
	// Normalizes to range [-1;1] keeping sign
	inline void normalize_tanh(Range original_range, std::vector<double>& series)
	{
		if (!original_range.is_valid())
		{
			throw std::runtime_error("normalize_tanh: original range is nan or less than epsilon");
		}
		if (abs(original_range.max()) > abs(original_range.min()))
		{
			assert(original_range.max() > 0.);
			original_range = Range{-original_range.max(), original_range.max()};
		}
		else
		{
			assert(original_range.min() < 0.);
			original_range = Range{original_range.min(), -original_range.min()};
		}
		normalize_range(original_range, Range{-1., 1.}, series);
	}

	//-----------------------------------------------------------------------------------------------------
	// class Normalization
	//-----------------------------------------------------------------------------------------------------
	class Normalization
	{
	public:
		virtual void normalize(std::vector<double>& series) = 0;
		virtual double restore(double normalized_val) const = 0;
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
		virtual void normalize(std::vector<double>& series) override;
		virtual double restore(double normalized_val) const override;
	};
	//-----------------------------------------------------------------------------------------------------
	inline void RangeNormalization::normalize(std::vector<double>& series)
	{
		original_range_ = min_max(series);
		normalize_range(original_range_, normal_range_, series);
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
		virtual void normalize(std::vector<double>& series) override;
		virtual double restore(double normalized_val) const override;
	};
	//-----------------------------------------------------------------------------------------------------
	inline void Lognormal::normalize(std::vector<double>& series)
	{
		std::transform(std::cbegin(series), std::cend(series), std::begin(series),
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