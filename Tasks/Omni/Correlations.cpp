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
void Correlations::calculate(const DataFrame& df)
{
	dimention_ = df.col_count();
	const size_t row_count = df.row_count();
	matrix_.resize(dimention_ * dimention_, 0.);
	if (!dimention_)
	{
		return;
	}

	std::vector<__int64> valid_count(dimention_, 0);
	std::vector<double> valid_mean(dimention_, 0.);

	{
		for (std::size_t col_idx = 0; col_idx != dimention_; ++col_idx)
		{
			const DataFrame::series_t& series = df.data()[col_idx];
			for (std::size_t row_idx = 0; row_idx < row_count; ++row_idx)
			{
				if (!std::isnan(series[row_idx]))
				{
					++valid_count[col_idx];
					valid_mean[col_idx] += series[row_idx];
				}
			}
			if (valid_count[col_idx])
			{
				valid_mean[col_idx] /= valid_count[col_idx];
			}
		}
	}
	{
		Matrix covariance(dimention_ * dimention_, 0.);
		Matrix squared_dev(dimention_, 0.);

		for (size_t x_idx = 0; x_idx != dimention_; ++x_idx)
		{
			const DataFrame::series_t& x_series = df.data()[x_idx];
			for (size_t y_idx = x_idx; y_idx != dimention_; ++y_idx)
			{
				const DataFrame::series_t& y_series = df.data()[y_idx];
				covariance[x_idx * dimention_ + y_idx] = 0.;
				squared_dev[y_idx] = 0.;
				__int64 valid_count_xy = 0;
				for (size_t i = 0; i < row_count; ++i)
				{
					if (!std::isnan(x_series[i]) && !std::isnan(y_series[i]))
					{
						++valid_count_xy;
						covariance[x_idx * dimention_ + y_idx] += (x_series[i] - valid_mean[x_idx]) * (y_series[i] - valid_mean[y_idx]);
						squared_dev[y_idx] += (y_series[i] - valid_mean[y_idx]) * (y_series[i] - valid_mean[y_idx]);
					}
				}
				if (squared_dev[x_idx] && squared_dev[y_idx])
				{
					matrix_[x_idx * dimention_ + y_idx] = covariance[x_idx * dimention_ + y_idx] / std::sqrt(squared_dev[x_idx] * squared_dev[y_idx]);
				}
			}
		}
	}
}
