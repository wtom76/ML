#include "stdafx.h"
#include <QStringList>
#include "ColumnPath.h"

//----------------------------------------------------------------------------------------------------------
QString ColumnPath::toString() const
{
	return schema_ + "/" + table_ + "/" + column_ + "/" + data_type_;
}
//----------------------------------------------------------------------------------------------------------
ColumnPath ColumnPath::fromString(const QString& src)
{
	ColumnPath result;
	QStringList lst = src.split('/', QString::KeepEmptyParts);
	if (lst.size() < 4)
	{
		return result;
	}
	result.schema_		= lst[0];
	result.table_		= lst[1];
	result.column_		= lst[2];
	result.data_type_	= lst[3];
	return result;
}
