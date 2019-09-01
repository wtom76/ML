#pragma once
#include <QString>

struct ColumnInfo;

//-------------------------------------------------------------------------------------------------
// struct ColumnPath
//-------------------------------------------------------------------------------------------------
struct ColumnPath
{
	QString schema_;
	QString table_;
	QString column_;
	QString data_type_;

	QString toString() const;
	static ColumnPath fromString(const QString& src);
};
