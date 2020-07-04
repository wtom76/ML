#pragma once
#include <limits>
#include <Shared/DbAccess/DbAccess.h>
#include <Shared/Math/Statistics.hpp>
#include <Shared/Utility/types.hpp>

//----------------------------------------------------------------------------------------------------------
// class ColumnMetrics
//----------------------------------------------------------------------------------------------------------
class ColumnMetrics
{
public:
	__int64 valid_count_{};
	__int64 null_count_{};
	wtom::ml::math::Range min_max_{std::numeric_limits<double>::quiet_NaN(), std::numeric_limits<double>::quiet_NaN()};

public:
	explicit ColumnMetrics(DataFrame::series_t& data);
	~ColumnMetrics();
};

