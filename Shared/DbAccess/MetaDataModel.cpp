#include "stdafx.h"
#include <QSqlQuery>
#include <QSqlError>
#include <QMessageBox>
#include <QApplication>
#include <Shared/DbAccess/DbAccess.h>
#include "MetaDataModel.h"

const char* const dest_schema = "ready";
const char* const dest_table = "daily_0001";

//----------------------------------------------------------------------------------------------------------
MetaDataModel::MetaDataModel(DbAccess& db, QObject* parent)
	: QAbstractTableModel(parent)
	, col_names_{"table", "column", "description", "origin", "normalized", "norm_min", "norm_max", "unit_id"}
	, db_(db)
{
	load();
}
//----------------------------------------------------------------------------------------------------------
MetaDataModel::~MetaDataModel()
{
}
//---------------------------------------------------------------------------------------------------------
void MetaDataModel::load()
{
	beginResetModel();
	data_ = db_.loadMetaData();
	endResetModel();
}
//---------------------------------------------------------------------------------------------------------
int MetaDataModel::rowCount(const QModelIndex& parent) const
{
	return parent.isValid() ? 0 : static_cast<int>(data_.size());
}
//---------------------------------------------------------------------------------------------------------
int MetaDataModel::columnCount(const QModelIndex& parent) const
{
	return parent.isValid() ? 0 : column_count_;
}
//---------------------------------------------------------------------------------------------------------
QVariant MetaDataModel::data(const QModelIndex& index, int role) const
{
	switch (role)
	{
	case Qt::DisplayRole:
		assert(index.column() < column_count_);
		{
			assert(index.row() < data_.size());
			switch (index.column())
			{
			case 0:	return data_[index.row()].table_;
			case 1:	return data_[index.row()].column_;
			case 2:	return data_[index.row()].description_;
			case 3:	return data_[index.row()].origin_;
			case 4:	return data_[index.row()].normalized_;
			case 5:	return data_[index.row()].norm_min_;
			case 6:	return data_[index.row()].norm_max_;
			case 7:	return data_[index.row()].unit_id_;
			}
		}
	}
	return QVariant();
}
//---------------------------------------------------------------------------------------------------------
QVariant MetaDataModel::headerData(int section, Qt::Orientation orientation, int role) const
{
	switch (orientation)
	{
	case Qt::Horizontal:
		switch (role)
		{
		case Qt::DisplayRole:
			return col_names_[section];
		}
	}
	return QAbstractTableModel::headerData(section, orientation, role);
}
//----------------------------------------------------------------------------------------------------------
void MetaDataModel::addColumn(const ColumnPath& path, int unit_id)
{
	std::set<QString> existing_cols = db_.tableColumns(dest_schema, dest_table);
	if (existing_cols.find(path.column_) != existing_cols.cend())
	{
		return;
	}
	db_.addColumn(dest_schema, dest_table, unit_id, path);
	load();
}
//----------------------------------------------------------------------------------------------------------
void MetaDataModel::deleteColumn(int idx)
{
	assert(idx >= 0);
	assert(idx < data_.size());

	if (data_[idx].column_ == "date")
	{
		QMessageBox::information(qApp->activeWindow(), "Column deletion is rejected", "'data' column should not be deleted");
	}
	db_.deleteColumn(dest_schema, dest_table, data_[idx]);
	load();
}
//----------------------------------------------------------------------------------------------------------
std::vector<ColumnInfo> MetaDataModel::columnInfos() const noexcept
{
	return data_;
}
