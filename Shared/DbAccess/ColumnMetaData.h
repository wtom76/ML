#pragma once
#include <string>
#include <vector>

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
};
//----------------------------------------------------------------------------------------------------------
inline std::vector<std::string> col_names(const std::vector<ColumnMetaData>& metas)
{
	std::vector<std::string> result;
	result.reserve(metas.size());
	std::transform(metas.cbegin(), metas.cend(), std::back_inserter(result), [](const auto& meta) noexcept { return meta.column_; });
	return result;
}