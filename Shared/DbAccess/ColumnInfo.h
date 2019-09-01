#pragma once
#include <QString>

//----------------------------------------------------------------------------------------------------------
// struct ColumnInfo
//----------------------------------------------------------------------------------------------------------
struct ColumnInfo
{
	int id_ = 0;
	QString table_;
	QString column_;
	QString description_;
	QString origin_;
	bool normalized_ = false;
	double norm_min_ = 0.;
	double norm_max_ = 0.;
	long long unit_id_ = 0;
};
