#pragma once

#include <set>
#include <vector>
#include <string>
#include <QtCore/QObject>
#include <QtSql/QSqlDatabase>
#include <Shared/Utility/types.hpp>
#include <Shared/DbAccess/ColumnPath.h>
#include <Shared/DbAccess/ColumnMetaData.h>

struct ColumnPath;

//----------------------------------------------------------------------------------------------------------
// struct UnitInfo
//----------------------------------------------------------------------------------------------------------
struct UnitInfo
{
	int id_ = 0;
	std::string name_;

	template <typename NameT>
	UnitInfo(int id, NameT&& name) : id_(id), name_(std::forward<NameT>(name))
	{}
};
//----------------------------------------------------------------------------------------------------------
// class DbAccess
//----------------------------------------------------------------------------------------------------------
class DbAccess
	: public QObject
{
	Q_OBJECT;

	class Impl;

	std::unique_ptr<Impl> impl_;
	std::string dest_schema_{"ready"};

public:
	DbAccess();
	~DbAccess();

	const std::string& dest_schema() const noexcept { return dest_schema_; }
	void set_dest_schema(const std::string& value) { dest_schema_ = value; }

	std::set<std::string> tableColumns(const std::string& schema_name, const std::string& table_name) const;
	void add_column(const std::string& dest_schema_name, const ColumnMetaData& col_meta);
	void copy_column_data(const std::string& dest_schema_name, const std::string& dest_table_name, const ColumnPath& col_info);
	void delete_column(const std::string& dest_schema_name, const ColumnMetaData& col_info);
	std::vector<UnitInfo> load_units() const;
	std::vector<ColumnMetaData> load_meta_data() const;
	void store_column(const ColumnMetaData& col_info, const DataFrame& data) const;
	void store_column_metadata(const ColumnMetaData& col_info) const;
	void store_column_metadata_is_target(const ColumnMetaData& col_info) const;
	DataFrame load_data(const std::string& schema, const std::string& table_name, const std::vector<std::string>& col_names) const;
};
