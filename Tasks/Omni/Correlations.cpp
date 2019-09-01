#include "stdafx.h"
#include <Shared/DbAccess/DbAccess.h>
#include "Correlations.h"

//----------------------------------------------------------------------------------------------------------
Correlations::Correlations(const std::vector<ColumnData>& data)
{
	_loadCalc(data);
}
//----------------------------------------------------------------------------------------------------------
Correlations::~Correlations()
{
}
//----------------------------------------------------------------------------------------------------------
void Correlations::_loadCalc(const std::vector<ColumnData>& data)
{
	matrix_.resize(data.size() * data.size(), 0.);

	if (data.empty())
	{
		return;
	}

	std::vector<__int64> valid_count(data.size(), 0);
	std::vector<double> valid_mean(data.size(), 0.);

	{
		for (std::size_t col_idx = 0; col_idx < data.size(); ++col_idx)
		{
			for (std::size_t row_idx = 0; row_idx < data[col_idx].values_.size(); ++row_idx)
			{
				if (data[col_idx].valid_[row_idx])
				{
					++valid_count[col_idx];
					valid_mean[col_idx] += data[col_idx].values_[row_idx];
				}
			}
			if (valid_count[col_idx])
			{
				valid_mean[col_idx] /= valid_count[col_idx];
			}
		}
	}
	{
		Matrix covariance(data.size() * data.size(), 0.);
		Matrix squared_dev(data.size(), 0.);

		for (std::size_t x_idx = 0; x_idx < data.size()/* - 1*/; ++x_idx)
		{
			for (std::size_t y_idx = x_idx/* + 1*/; y_idx < data.size(); ++y_idx)
			{
				covariance[x_idx * data.size() + y_idx] = 0.;
				squared_dev[y_idx] = 0.;
				__int64 valid_count_xy = 0;
				for (std::size_t i = 0; i < data[x_idx].values_.size(); ++i)
				{
					if (data[x_idx].valid_[i] && data[y_idx].valid_[i])
					{
						++valid_count_xy;
						covariance[x_idx * data.size() + y_idx] += (data[x_idx].values_[i] - valid_mean[x_idx]) * (data[y_idx].values_[i] - valid_mean[y_idx]);
						squared_dev[y_idx] += (data[y_idx].values_[i] - valid_mean[y_idx]) * (data[y_idx].values_[i] - valid_mean[y_idx]);
					}
				}
				if (squared_dev[x_idx] && squared_dev[y_idx])
				{
					matrix_[x_idx * data.size() + y_idx] = covariance[x_idx * data.size() + y_idx] / std::sqrt(squared_dev[x_idx] * squared_dev[y_idx]);
				}
			}
		}
	}
}
