#pragma once

#include <set>
#include <vector>
#include <string>
#include <QObject>
#include <QSqlDatabase>
#include <Shared/Utility/types.hpp>
#include <Shared/DbAccess/ColumnPath.h>
#include <Shared/DbAccess/ColumnMetaData.h>

extern const std::string g_dest_schema;
extern const std::string g_dest_table;

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

public:
	DbAccess();
	~DbAccess();

	std::set<std::string> tableColumns(const std::string& schema_name, const std::string& table_name) const;
	void add_column(const std::string& dest_schema_name, const ColumnMetaData& col_meta);
	void copy_column_data(const std::string& dest_schema_name, const std::string& dest_table_name, const ColumnPath& col_info);
	void delete_column(const std::string& dest_schema_name, const std::string& dest_table_name, const ColumnMetaData& col_info);
	std::vector<UnitInfo> load_units() const;
	std::vector<ColumnMetaData> load_meta_data() const;
	void store_column(const ColumnMetaData& col_info, const DataFrame& data) const;
	DataFrame load_data(const std::string& schema, const std::string& table_name, const std::vector<std::string>& col_names) const;
};
