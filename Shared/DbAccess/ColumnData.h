#pragma once
#include <vector>

//----------------------------------------------------------------------------------------------------------
// struct ColumnData
//----------------------------------------------------------------------------------------------------------
struct ColumnData
{
	std::vector<QDate>	dates_;
	std::vector<double>	values_;
	std::vector<bool>	valid_;
};
