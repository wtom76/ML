#pragma once
#include <QString>

struct ColumnMetaData;

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
