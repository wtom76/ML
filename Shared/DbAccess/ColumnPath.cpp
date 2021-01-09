#include "stdafx.h"
#include <string>
#include <vector>
#include <sstream>
#include "ColumnPath.h"

//----------------------------------------------------------------------------------------------------------
std::string ColumnPath::to_string() const
{
	return schema_ + "/" + table_ + "/" + column_ + "/" + data_type_;
}
//----------------------------------------------------------------------------------------------------------
ColumnPath ColumnPath::from_string(const std::string& src)
{
	ColumnPath result;
	vector<string> lst;
	stringstream sstrm(src);
	string part;
	while (getline(sstrm, part, '/'))
	{
		lst.emplace_back(std::move(part));
	}

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
