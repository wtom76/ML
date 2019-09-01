#include "stdafx.h"
#include "ColumnMetrics.h"

//----------------------------------------------------------------------------------------------------------
ColumnMetrics::ColumnMetrics(const ColumnData& col_data)
{
	for (size_t i = 0; i < col_data.values_.size(); ++i)
	{
		if (col_data.valid_[i])
		{
			++valid_count_;
			if (max_ < col_data.values_[i])
			{
				max_ = col_data.values_[i];
			}
			if (min_ > col_data.values_[i])
			{
				min_ = col_data.values_[i];
			}
		}
		else
		{
			++null_count_;
		}
	}
}
//----------------------------------------------------------------------------------------------------------
ColumnMetrics::~ColumnMetrics()
{
}
