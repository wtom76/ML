#include "stdafx.h"
#include "UpdateDates.h"
#include "../DbOperation.h"

//---------------------------------------------------------------------------------------------------------
operation::UpdateDates::~UpdateDates()
{}
//---------------------------------------------------------------------------------------------------------
std::string operation::UpdateDates::name() const
{
	return "Update dates"s;
}
//---------------------------------------------------------------------------------------------------------
std::string operation::UpdateDates::description() const
{
	return "Update metadata with series dates boundaies"s;
}
//---------------------------------------------------------------------------------------------------------
void operation::UpdateDates::run(DbOperation* ctx)
{
	MetaDataModel& model{ctx->meta_model()};
	const vector<int> col_idxs{model.selected_columns()};
	for (int idx : col_idxs)
	{
		ColumnMetaData& col_meta{model.col_meta(idx)};
		const DataFrame col_data{model.load_column(idx)};
		pair<system_clock::time_point, system_clock::time_point> min_max;
		if (_date_boundary(col_data, min_max) &&
			(col_meta.date_min_ != min_max.first ||
			 col_meta.date_max_ != min_max.second))
		{
			col_meta.date_min_ = min_max.first;
			col_meta.date_max_ = min_max.second;
			model.store_col_meta(idx);
		}
	}
}
//---------------------------------------------------------------------------------------------------------
bool operation::UpdateDates::_date_boundary(const DataFrame& data, pair<system_clock::time_point, system_clock::time_point>& min_max)
{
	const DataFrame::index_series_t& index{data.index()};
	if (index.empty())
	{
		return false;
	}
	min_max.first = min_max.second = index[0];
	for (system_clock::time_point dt : index)
	{
		if (min_max.first > dt)
		{
			min_max.first = dt;
		}
		if (min_max.second < dt)
		{
			min_max.second = dt;
		}
	}
	return true;
}
