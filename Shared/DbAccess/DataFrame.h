#pragma once
#include <vector>
#include <chrono>
#include "IncludeDataFrame.h"

////----------------------------------------------------------------------------------------------------------
//// struct DataFrame
////----------------------------------------------------------------------------------------------------------
//struct DataFrame
//{
//	std::vector<std::chrono::system_clock::time_point> dates_;
//	std::vector<double>	values_;
//	std::vector<bool>	valid_;
//};

using DataFrame = hmdf::StdDataFrame<hmdf::DateTime>;