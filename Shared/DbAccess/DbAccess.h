#pragma once

#include <set>
#include <vector>
#include <string>
#include <QObject>
#include <QSqlDatabase>
#include <Shared/DbAccess/ColumnPath.h>
#include <Shared/DbAccess/DataFrame.h>
#include <Shared/DbAccess/ColumnMetaData.h>

struct ColumnPath;

//----------------------------------------------------------------------------------------------------------
// struct UnitInfo
//----------------------------------------------------------------------------------------------------------
struct UnitInfo
{
	int id_ = 0;
	std::string name_;

//	UnitInfo() = default;
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
	void addColumn(const std::string& dest_schema_name, const std::string& dest_table_name, int unit_id, const ColumnPath& col_info);
	void deleteColumn(const std::string& dest_schema_name, const std::string& dest_table_name, const ColumnMetaData& col_info);
	std::vector<UnitInfo> loadUnits() const;
	std::vector<ColumnMetaData> loadMetaData() const;
	DataFrame load_data(const std::string& schema, const std::string& table_name, const std::vector<std::string>& col_names) const;
	void storeColumnData(const ColumnMetaData& col_info, const DataFrame& data) const;
};
