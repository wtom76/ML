#include "stdafx.h"
#include <Shared/DbAccess/ColumnMetaData.h>
#include <Shared/DbAccess/ColumnPath.h>

using namespace std;

//----------------------------------------------------------------------------------------------------------
ColumnMetaData::ColumnMetaData(
	const string& dest_table_name,
	const std::string& dest_column_name,
	int unit_id,
	const ColumnPath& origin,
	bool is_target,
	double target_error)
	: table_{dest_table_name}
	, column_{dest_column_name}
	, unit_id_{unit_id}
	, origin_{origin.to_string()}
	, is_target_{is_target}
	, target_error_{target_error}
{}
//----------------------------------------------------------------------------------------------------------
ColumnPath ColumnMetaData::column_path() const
{
	return ColumnPath::from_string(origin_);
}
//----------------------------------------------------------------------------------------------------------
std::vector<std::string> col_names(const std::vector<ColumnMetaData>& metas)
{
	std::vector<std::string> result;
	result.reserve(metas.size());
	std::transform(metas.cbegin(), metas.cend(), std::back_inserter(result), [](const auto& meta) noexcept { return meta.column_; });
	return result;
}
