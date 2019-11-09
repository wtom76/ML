#include "stdafx.h"
#include "ColumnMetrics.h"
#include <Shared/Math/DataFrameVisitors.hpp>

using namespace std;

//----------------------------------------------------------------------------------------------------------
ColumnMetrics::ColumnMetrics(const string& col_name, DataFrame& col_data)
{
	const pair<size_t, size_t> shape = col_data.shape();
	MinMaxValidCounter<double, hmdf::DateTime> visitor{};
	col_data.visit<double>(col_name.c_str(), visitor);
	valid_count_ = visitor.valid_count();
	null_count_ = shape.first - valid_count_;
	if (valid_count_)
	{
		min_ = visitor.min();
		max_ = visitor.max();
	}
}
//----------------------------------------------------------------------------------------------------------
ColumnMetrics::~ColumnMetrics()
{
}
