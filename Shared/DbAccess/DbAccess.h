#pragma once

#include <set>
#include <vector>
#include <QObject>
#include <QSqlDatabase>
#include <Shared/DbAccess/ColumnPath.h>
#include <Shared/DbAccess/ColumnData.h>
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
class DbAccess : public QObject
{
	Q_OBJECT;

public:
	DbAccess();
	~DbAccess();

	std::set<QString> tableColumns(const QString& schema_name, const QString& table_name) const;
	void addColumn(const QString& dest_schema_name, const QString& dest_table_name, int unit_id, const ColumnPath& col_info);
	void deleteColumn(const QString& dest_schema_name, const QString& dest_table_name, const ColumnMetaData& col_info);
	std::vector<UnitInfo> loadUnits() const;
	std::vector<ColumnMetaData> loadMetaData() const;
	ColumnData loadColumnData(const QString& schema, const QString& table, const QString& column) const;
	void storeColumnData(const ColumnMetaData& col_info, const ColumnData& data) const;
};
