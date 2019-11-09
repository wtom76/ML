#pragma once
#include <limits>
#include <Shared/DbAccess/DbAccess.h>

//----------------------------------------------------------------------------------------------------------
// class ColumnMetrics
//----------------------------------------------------------------------------------------------------------
class ColumnMetrics
{
public:
	__int64 valid_count_{};
	__int64 null_count_{};
	double max_ = std::numeric_limits<double>::min();
	double min_ = std::numeric_limits<double>::max();

public:
	explicit ColumnMetrics(const std::string& col_name, DataFrame& col_data);
	~ColumnMetrics();
};

