#include "stdafx.h"
#include "Normalizer.h"
#include <Shared/Math/DataFrameVisitors.hpp>

using namespace std;

//----------------------------------------------------------------------------------------------------------
Normalizer::Normalizer()
{
}
//----------------------------------------------------------------------------------------------------------
Normalizer::~Normalizer()
{
}
//----------------------------------------------------------------------------------------------------------
bool Normalizer::normalize(const string& col_name, DataFrame& data, pair<double, double>& min_max) const
{
	MinMaxValidCounter<double, hmdf::DateTime> visitor{};
	data.visit<double>(col_name.c_str(), visitor);
	if (!visitor.valid_count())
	{
		return false;
	}
	double min = visitor.min();
	double max = visitor.max();

	if (max - min < numeric_limits<double>::epsilon())
	{
		return false;
	}

	auto& column = data.get_column<double>(col_name.c_str());
	auto value_i = column.begin();
	auto value_ce = column.cend();
	for (; value_i != value_ce; ++value_i)
	{
		if (!hmdf::is_nan__(*value_i))
		{
			*value_i = (*value_i - min) / (max - min);
		}
	}

	min_max.first = min;
	min_max.second = max;

	return true;
}
