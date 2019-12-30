#pragma once
#include <string>
#include <vector>
#include <optional>
#include <chrono>

struct ColumnPath;

//----------------------------------------------------------------------------------------------------------
// struct ColumnMetaData
//----------------------------------------------------------------------------------------------------------
struct ColumnMetaData
{
	long long id_ = 0;
	std::string table_;
	std::string column_;
	std::string description_;
	std::string origin_;
	bool normalized_ = false;
	double norm_min_ = 0.;
	double norm_max_ = 0.;
	long long unit_id_ = 0;
	std::optional<std::chrono::system_clock::time_point> date_min_{};
	std::optional<std::chrono::system_clock::time_point> date_max_{};

	ColumnMetaData() = default;
	ColumnMetaData(const std::string& dest_table_name, const std::string& dest_column_name, int unit_id, const ColumnPath& origin);
	ColumnPath column_path() const;
};
//----------------------------------------------------------------------------------------------------------
std::vector<std::string> col_names(const std::vector<ColumnMetaData>& metas);