#include "stdafx.h"
#include "Normalizer.h"

//----------------------------------------------------------------------------------------------------------
Normalizer::Normalizer()
{
}
//----------------------------------------------------------------------------------------------------------
Normalizer::~Normalizer()
{
}
//----------------------------------------------------------------------------------------------------------
bool Normalizer::normalize(ColumnData& data, std::pair<double, double>& min_max) const
{
	bool normalized = false;
	double min = std::numeric_limits<double>::max();
	double max = -std::numeric_limits<double>::max();

	auto value_ci = data.values_.cbegin();
	const auto value_ce = data.values_.cend();
	auto valid_ci = data.valid_.cbegin();
	for (; value_ci != value_ce; ++value_ci, ++valid_ci)
	{
		if (*valid_ci)
		{
			normalized = true;
			if (min > *value_ci)
			{
				min = *value_ci;
			}
			if (max < *value_ci)
			{
				max = *value_ci;
			}
		}
	}

	if (max - min < std::numeric_limits<double>::epsilon())
	{
		return false;
	}

	auto value_i = data.values_.begin();
	valid_ci = data.valid_.cbegin();
	for (; value_i != value_ce; ++value_i, ++valid_ci)
	{
		if (*valid_ci)
		{
			*value_i = (*value_i - min) / (max - min);
		}
	}

	min_max.first = min;
	min_max.second = max;

	return true;
}
