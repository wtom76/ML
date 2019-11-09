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

	//std::vector<__int64> valid_count(dimention_, 0);
	//std::vector<double> valid_mean(dimention_, 0.);

	//{
	//	for (std::size_t col_idx = 0; col_idx < dimention_; ++col_idx)
	//	{
	//		for (std::size_t row_idx = 0; row_idx < data[col_idx].values_.size(); ++row_idx)
	//		{
	//			if (data[col_idx].valid_[row_idx])
	//			{
	//				++valid_count[col_idx];
	//				valid_mean[col_idx] += data[col_idx].values_[row_idx];
	//			}
	//		}
	//		if (valid_count[col_idx])
	//		{
	//			valid_mean[col_idx] /= valid_count[col_idx];
	//		}
	//	}
	//}
	//{
	//	Matrix covariance(data.size() * data.size(), 0.);
	//	Matrix squared_dev(data.size(), 0.);

	//	for (std::size_t x_idx = 0; x_idx < data.size()/* - 1*/; ++x_idx)
	//	{
	//		for (std::size_t y_idx = x_idx/* + 1*/; y_idx < data.size(); ++y_idx)
	//		{
	//			covariance[x_idx * data.size() + y_idx] = 0.;
	//			squared_dev[y_idx] = 0.;
	//			__int64 valid_count_xy = 0;
	//			for (std::size_t i = 0; i < data[x_idx].values_.size(); ++i)
	//			{
	//				if (data[x_idx].valid_[i] && data[y_idx].valid_[i])
	//				{
	//					++valid_count_xy;
	//					covariance[x_idx * data.size() + y_idx] += (data[x_idx].values_[i] - valid_mean[x_idx]) * (data[y_idx].values_[i] - valid_mean[y_idx]);
	//					squared_dev[y_idx] += (data[y_idx].values_[i] - valid_mean[y_idx]) * (data[y_idx].values_[i] - valid_mean[y_idx]);
	//				}
	//			}
	//			if (squared_dev[x_idx] && squared_dev[y_idx])
	//			{
	//				matrix_[x_idx * data.size() + y_idx] = covariance[x_idx * data.size() + y_idx] / std::sqrt(squared_dev[x_idx] * squared_dev[y_idx]);
	//			}
	//		}
	//	}
	//}
}
