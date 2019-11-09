#include "stdafx.h"
#include <QSqlQuery>
#include <QSqlError>
#include <QMessageBox>
#include <QApplication>
#include <Shared/DbAccess/DbAccess.h>
#include "CorrelationsModel.h"

using namespace std;

const char* const dest_schema = "ready";
const char* const dest_table = "daily_0001";

//----------------------------------------------------------------------------------------------------------
CorrelationsModel::CorrelationsModel(const std::vector<ColumnMetaData>& col_infos, DbAccess& db, QObject* parent)
	: QAbstractTableModel(parent)
	, col_infos_{col_infos}
	, db_(db)
{
	_load();
}
//----------------------------------------------------------------------------------------------------------
CorrelationsModel::~CorrelationsModel()
{
}
//---------------------------------------------------------------------------------------------------------
void CorrelationsModel::_load()
{
	beginResetModel();
	vector<string> normalized_cols;
	string table_name;
	for (const auto& col_info : col_infos_)
	{
		if (col_info.normalized_)
		{
			if (table_name.empty())
			{
				table_name = col_info.table_;
			}
			else
			{
				assert(table_name == col_info.table_);		// different tables are not supported
			}
			normalized_cols.emplace_back(col_info.column_);
		}
	}
	DataFrame data = db_.load_data("ready", table_name, normalized_cols);
	data_.calculate(col_names(col_infos_), data);
	endResetModel();
}
//---------------------------------------------------------------------------------------------------------
int CorrelationsModel::rowCount(const QModelIndex& parent) const
{
	return parent.isValid() ? 0 : static_cast<int>(data_.dimention());
}
//---------------------------------------------------------------------------------------------------------
int CorrelationsModel::columnCount(const QModelIndex& parent) const
{
	return parent.isValid() ? 0 : static_cast<int>(data_.dimention());
}
//---------------------------------------------------------------------------------------------------------
QVariant CorrelationsModel::data(const QModelIndex& index, int role) const
{
	switch (role)
	{
	case Qt::DisplayRole:
		assert(index.row() >= 0 && index.row() < data_.dimention());
		assert(index.column() >= 0 && index.column() < data_.dimention());
		return data_.value(index.row(), index.column());
	}
	return QVariant();
}
//---------------------------------------------------------------------------------------------------------
QVariant CorrelationsModel::headerData(int section, Qt::Orientation orientation, int role) const
{
	switch (orientation)
	{
	case Qt::Horizontal:
	case Qt::Vertical:
		switch (role)
		{
		case Qt::DisplayRole:
			return QString::fromStdString(col_infos_[section].column_);
		}
	}
	return QAbstractTableModel::headerData(section, orientation, role);
}
