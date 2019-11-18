#include "stdafx.h"
#include <Shared/DbAccess/DbAccess.h>
#include "Correlations.h"

//----------------------------------------------------------------------------------------------------------
Correlations::Correlations()
{
}
//----------------------------------------------------------------------------------------------------------
Correlations::~Correlations()
{
}
//----------------------------------------------------------------------------------------------------------
void Correlations::calculate(const std::vector<std::string>& col_names, const DataFrame& data)
{
	dimention_ = col_names.size();
	matrix_.resize(dimention_ * dimention_, 0.);

	if (!dimention_)
	{
		return;
	}

	for (std::size_t x_idx = 0; x_idx < dimention_/* - 1*/; ++x_idx)
	{
		for (std::size_t y_idx = x_idx/* + 1*/; y_idx < dimention_; ++y_idx)
		{
			hmdf::CorrVisitor<double, hmdf::DateTime> corr{};
			data.visit<double, double>(col_names[x_idx].c_str(), col_names[y_idx].c_str(), corr);
			matrix_[x_idx * dimention_ + y_idx] = corr.get_result();
		}
	}
}
