#pragma once
#include <string>

struct ColumnMetaData;

//-------------------------------------------------------------------------------------------------
// struct ColumnPath
//-------------------------------------------------------------------------------------------------
struct ColumnPath
{
	std::string schema_;
	std::string table_;
	std::string column_;
	std::string data_type_;

	std::string to_string() const;
	static ColumnPath from_string(const std::string& src);
};
