#include "stdafx.h"
#include "ColumnMetrics.h"
#include <Shared/Data/Frame.hpp>

using namespace std;

//----------------------------------------------------------------------------------------------------------
ColumnMetrics::ColumnMetrics(DataFrame::series_t* data)
{
	if (!data || data)
	{
		return;
	}
	size_t row_count = data->size();
	if (!row_count)
	{
		return;
	}
	size_t nan_count = 0;
	min_max_ = wtom::ml::math::min_max_nan(*data, nan_count);
	assert(row_count >= nan_count);
	valid_count_ = row_count - nan_count;
	null_count_ = nan_count;
}
//----------------------------------------------------------------------------------------------------------
ColumnMetrics::~ColumnMetrics()
{
}
