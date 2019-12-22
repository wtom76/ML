#pragma once
#include <vector>
#include <chrono>
#include "IncludeDataFrame.h"

using DataFrame = hmdf::StdDataFrame<hmdf::DateTime>;
using DataColumn = typename hmdf::type_declare<hmdf::HeteroVector, double>::type;