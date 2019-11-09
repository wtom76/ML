#pragma once

#include <Shared/DbAccess/DataFrame.h>

//----------------------------------------------------------------------------------------------------------
// class Normalizer
//----------------------------------------------------------------------------------------------------------
class Normalizer
{
public:
	Normalizer();
	~Normalizer();

	bool normalize(const std::string& col_name, DataFrame& data, std::pair<double, double>& min_max) const;
};
