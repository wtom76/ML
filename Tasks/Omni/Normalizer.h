#pragma once

#include <Shared/DbAccess/ColumnData.h>

//----------------------------------------------------------------------------------------------------------
// class Normalizer
//----------------------------------------------------------------------------------------------------------
class Normalizer
{
public:
	Normalizer();
	~Normalizer();

	bool normalize(ColumnData& data, std::pair<double, double>& min_max) const;
};
